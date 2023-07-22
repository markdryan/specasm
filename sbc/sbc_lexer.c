/*
 * Copyright contributors to Specasm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#include "sbc_error.h"
#include "sbc_lexer.h"

sbc_lexer_state_t lex;

void sbc_lexer_open_e(const char *f)
{
	lex.h = specasm_file_ropen_e(f);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SBC_ERROR_OPEN;
		return;
	}
	lex.start = 0;
	lex.end = 0;
	lex.eof = 0;
}

static void prv_ensure_buffer_e(uint8_t wanted)
{
	uint16_t to_read;
	uint16_t read;
	uint16_t bytes_in_buffer = lex.end - lex.start;

	if (wanted <= bytes_in_buffer)
		return;

	if (lex.eof) {
		err_type = SBC_ERROR_BAD_PROGRAM;
		return;
	}

	to_read = SBC_LEX_BUF_SIZE - bytes_in_buffer;

	/*
	 * src and destination cannot overlap as the buffer is 4 * the maximum
	 * token size.
	 */

	if ((bytes_in_buffer > 0) && (lex.start > 0))
		memcpy(lex.lex_buf,
		       &lex.lex_buf[lex.start],
		       bytes_in_buffer);
	read = specasm_file_read_e(lex.h,
				   &lex.lex_buf[bytes_in_buffer],
				   to_read);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SBC_ERROR_READ;
		return;
	}
	if (read < to_read)
		lex.eof = 1;
	lex.start = 0;
	lex.end = bytes_in_buffer + read;
	if (read < wanted) {
		err_type = SBC_ERROR_BAD_PROGRAM;
		return;
	}
}

static void prv_handle_line_start_e(void)
{

	/*
	 * Start of a new line.  Check to see if it's the last
	 * line.
	 */

	lex.start++;

	prv_ensure_buffer_e(1);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (lex.lex_buf[lex.start] == 0xff) {
		lex.tok.type = SBC_TOKEN_EOF;
		return;
	}

	prv_ensure_buffer_e(3);
	if (err_type != SPECASM_ERROR_OK)
		return;
	lex.tok.type = SBC_TOKEN_LINE_LABEL;
	lex.tok.tok.line_no =
		((uint16_t )lex.lex_buf[lex.start])
		<< 8;
	lex.start++;
	lex.tok.tok.line_no |=
		((uint16_t )lex.lex_buf[lex.start]);
	lex.start++;

	lex.line_no = lex.tok.tok.line_no;

	/*
	 * This 3rd byte contains the length of the line. This is handy
	 * as we can ensure this now and then now have to ensure until
	 * we get to another line.
	 */

	prv_ensure_buffer_e(lex.lex_buf[lex.start] - 1);
	lex.start++;
}

static void prv_handle_identifier(void)
{
	uint8_t ch;
	uint8_t len = 0;

	lex.tok.type = SBC_TOKEN_IDENTIFIER;

	do {
		len++;
		lex.start++;
		ch = lex.lex_buf[lex.start];
	} while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		 (ch >= '0' && ch <= '9') || (ch == '_'));

	if (ch == '$') {
		lex.tok.tok.id_type = SBC_ID_TYPE_STR;
		len++;
		lex.start++;
	} else if (ch == '%') {
		lex.tok.tok.id_type = SBC_ID_TYPE_INT;
		len++;
		lex.start++;
	} else {
		lex.tok.tok.id_type = SBC_ID_TYPE_REAL;
	}
	lex.tok.len = len;
	lex.tok.ptr = lex.start - len;
}

static void prv_handle_ext_keyword_e(void)
{
	uint8_t key;
	uint8_t first = lex.lex_buf[lex.start];

	lex.start++;
	key = lex.lex_buf[lex.start];

	if (key < 0x8e) {
		err_type = SBC_ERROR_BAD_PROGRAM;
		return;
	}

	key -= 0x8e;

	if (first == 0xc8)
		key += SBC_KEYWORD_CASE;
	else if (first == 0xc7)
		key += SBC_KEYWORD_APPEND;
	else
		key += SBC_KEYWORD_SUM;

	lex.tok.type = SBC_TOKEN_KEYWORD;
	lex.tok.tok.keyword = key;
	lex.tok.ptr = lex.start;
	lex.tok.len = 1;
	lex.start++;
}

static void prv_handle_floating(int8_t f)
{
	float num;
	char *ptr;
	uint8_t bin_num[4];
	char *end_ptr = NULL;

	ptr = (char *) &lex.lex_buf[lex.start];

	/*
	 * This isn't ideal.  SDCC only supports floats and not doubles
	 * so we're going to lose some precision in our floating point
	 * constants as the Spectrum has 40 bit floats.  So we're just
	 * going to do a strtof and then convert the binary representation
	 * into one that is understood by the Spectrum.
	 */

	num = strtof(ptr, &end_ptr) * (float) f;

	lex.start += (end_ptr - ptr);
	memcpy(&bin_num, &num, sizeof(bin_num));

	/*
	 * Compute the exponent.  The bias is 128 instead of 127 so fix that
	 * up.
	 */

	lex.tok.tok.real.b[0] = bin_num[3] << 1;
	if (bin_num[2] & 0x80)
		lex.tok.tok.real.b[0]++;
	lex.tok.tok.real.b[0]++;

	/*
	 * Copy the mantissa.
	 */

	lex.tok.tok.real.b[1] = bin_num[2];
	lex.tok.tok.real.b[2] = bin_num[1];
	lex.tok.tok.real.b[3] = bin_num[0];
	lex.tok.tok.real.b[4] = 0;

	/*
	 * Copy the sign bit.
	 */

	lex.tok.tok.real.b[1] &= 0x7f;
	lex.tok.tok.real.b[1] |= bin_num[3] & 0x80;
	lex.tok.type = SBC_TOKEN_REAL;
}

static void prv_handle_decimal_e(int8_t f)
{
	const uint8_t *last;
	int32_t old_val;
	int32_t factor = f;
	int32_t val = 0;
	const uint8_t *start = &lex.lex_buf[lex.start];
	const uint8_t *ptr = start + 1;

	while ((*ptr >= '0') && (*ptr <= '9')) {
		ptr++;
	}
	if ((*ptr == '.') || (*ptr == 'E')) {
		prv_handle_floating(f);
		return;
	}

	last = ptr;
	--ptr;

	while (ptr >= start) {
		old_val = val;
		if (*ptr != '0')
			val += (*ptr - '0') * factor;
		factor *= 10;

		/*
		 * This is undefined behaviour but will probably work.
		 * Need to check on all supported platforms or come up
		 * with a better way of doing this.
		 */

		if (((f > 0) && (val < old_val)) ||
		    ((f < 0) && (val > old_val))) {
			err_type = SBC_ERROR_BAD_NUM;
			return;
		}
		--ptr;
	}
	lex.tok.type = SBC_TOKEN_INTEGER;
	lex.tok.tok.integer = val;
	lex.start += (last - start);
}

static void prv_handle_complex_op(void)
{
	uint8_t second;
	const uint8_t *ptr;
	uint8_t first = lex.lex_buf[lex.start];

	lex.tok.type = SBC_TOKEN_OPERATOR;
	lex.tok.len = 1;
	lex.tok.ptr = lex.start;
	lex.start++;
	second = lex.lex_buf[lex.start];

	switch (first) {
	case '<':
		if ((second == '<') || (second == '>')  || (second == '='))
			break;
		return;
	case '>':
		if (second == '>') {
			if (lex.lex_buf[lex.start + 1] == '>') {
				lex.tok.len++;
				lex.start++;
				break;
			}
		} else if (second == '=') {
			break;
		}
		return;
	case '+':
		if (second == '=')
			break;
		return;
	case '-':
		if (second == '=')
			break;

		ptr = &lex.lex_buf[lex.start];
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		if (*ptr >= '0' && *ptr <= '9') {
			lex.start += ptr -
				&lex.lex_buf[lex.start];
			prv_handle_decimal_e(-1);
			return;
		} else if (*ptr == '.') {
			lex.start--;
			prv_handle_floating(1);
			return;
		}
		return;
	default:
		return;
	}
	lex.tok.len++;
	lex.start++;
}

static void prv_handle_string_e(void)
{
	const uint8_t *ptr;
	uint8_t len;
	const uint8_t *start = &lex.lex_buf[lex.start];

	for (ptr = start + 1; *ptr != '"'; ptr++) {
		if (*ptr == 13) {
			err_type = SBC_ERROR_MISSING_QUOTE;
			return;
		}
	}

	len = (ptr - start);
	lex.tok.type = SBC_TOKEN_STRING;
	lex.tok.len = len - 1;
	lex.tok.ptr = lex.start + 1;
	lex.start += len + 1;
}

static void prv_handle_line_no_e(void)
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t num[2];
	const uint8_t *ptr = &lex.lex_buf[lex.start + 1];

	prv_ensure_buffer_e(3);
	if (err_type != SPECASM_ERROR_OK)
		return;

	a = (*ptr * 4);
	++ptr;
	b = *ptr ^ (a & 0xc0);
	a = a * 4;
	++ptr;
	c = *ptr ^ a;

	num[0] = b;
	num[1] = c;

	memcpy(&lex.tok.tok.line_no, &num, 2);
	lex.tok.type = SBC_TOKEN_LINE_NUMBER;
	lex.start += 4;
}

static int8_t prv_is_hex_digit(uint8_t d)
{
	if (d >= '0' && d <= '9')
		return d - '0';

	if (d >= 'a' && d <= 'f')
		return d - 'a' + 10;

	if (d >= 'A' && d <= 'F')
		return d - 'A' + 10;

	return -1;
}

static void prv_handle_hex_e(void)
{
	int8_t dig;
	const uint8_t *ptr = &lex.lex_buf[lex.start + 1];
	uint8_t count = 1;

	dig = prv_is_hex_digit(*ptr);
	if (dig == -1) {
		err_type = SBC_ERROR_BAD_NUM;
		return;
	}

	lex.tok.tok.integer = dig;
	ptr++;
	while ((dig = prv_is_hex_digit(*ptr)) != -1) {
		if (count == 8) {
			err_type = SBC_ERROR_BAD_NUM;
			return;
		}
		lex.tok.tok.integer <<= 4;
		lex.tok.tok.integer |= dig;
		count++;
		ptr++;
	}
	lex.tok.type = SBC_TOKEN_HEX;
	lex.start += count + 1;
}

static void prv_handle_bin_e(void)
{
	const uint8_t *ptr = &lex.lex_buf[lex.start + 1];
	uint8_t count = 1;

	if (*ptr != '0' && *ptr != '1') {
		err_type = SBC_ERROR_BAD_NUM;
		return;
	}

	lex.tok.tok.integer = *ptr - '0';
	ptr++;
	while (*ptr == '0' || *ptr == '1') {
		if (count == 32) {
			err_type = SBC_ERROR_BAD_NUM;
			return;
		}
		lex.tok.tok.integer <<= 1;
		lex.tok.tok.integer |= *ptr - '0';
		count++;
		ptr++;
	}
	lex.tok.type = SBC_TOKEN_BIN;
	lex.start += count + 1;
}

void sbc_lexer_get_token_e(void)
{
	uint8_t first;
	const char *ptr;
	const char *simple_ops = "/*()=,;^~?$!:";
	const char *complex_ops = "<>+-";

	prv_ensure_buffer_e(1);
	if (err_type != SPECASM_ERROR_OK)
		return;

	first = lex.lex_buf[lex.start];

	if (first == 13) {
		prv_handle_line_start_e();
		return;
	}

	while (first == ' ' || first == '\t') {
		lex.start++;
		first = lex.lex_buf[lex.start];
	}

	if (first == 0x8d) {
		prv_handle_line_no_e();
		return;
	}

	if (first >= 0xc6 && first <= 0xc8) {
		prv_handle_ext_keyword_e();
		return;
	}

	if (first == '&') {
		prv_handle_hex_e();
		return;
	}

	if (first == '%') {
		prv_handle_bin_e();
		return;
	}

	if (first >= '0' && first <= '9') {
		prv_handle_decimal_e(1);
		return;
	}

	if ((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z') ||
	    (first == '_')){
		prv_handle_identifier();
		return;
	}

	for (ptr = simple_ops; *ptr && *ptr != first; ++ptr);
	if (*ptr) {
		lex.tok.type = SBC_TOKEN_OPERATOR;
		lex.tok.len = 1;
		lex.tok.ptr = lex.start;
		lex.start++;
		return;
	}

	if (first == '.') {
		prv_handle_floating(1);
		return;
	}

	if (first == '"') {
		prv_handle_string_e();
		return;
	}

	for (ptr = complex_ops; *ptr && *ptr != first; ++ptr);
	if (*ptr) {
		prv_handle_complex_op();
		return;
	}

	if (first >= 127) {
		lex.tok.tok.keyword = first - 127;
		lex.start++;
		lex.tok.ptr = lex.start;
		if (lex.tok.tok.keyword == SBC_KEYWORD_REM) {
			lex.tok.type = SBC_TOKEN_REM;
			while (lex.lex_buf[lex.start] != 13)
				lex.start++;
			lex.tok.len = lex.start -
				lex.tok.ptr;
		} else {
			lex.tok.type = SBC_TOKEN_KEYWORD;
			lex.tok.len = 1;
		}
		return;
	}

	lex.tok.type = SBC_TOKEN_EOF;
}

void sbc_lexer_close(void)
{
	specasm_error_t old_err = err_type;

	specasm_file_close_e(lex.h);
	err_type = old_err;
}
