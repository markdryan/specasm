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
#include "sbc_overlay.h"

void sbc_lexer_open_e(const char *f)
{
	overlay.lex.h = specasm_file_ropen_e(f);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SBC_ERROR_OPEN;
		return;
	}
	overlay.lex.start = 0;
	overlay.lex.end = 0;
	overlay.lex.eof = 0;
}

static void prv_ensure_buffer_e(uint8_t wanted)
{
	uint16_t to_read;
	uint16_t read;
	uint16_t bytes_in_buffer = overlay.lex.end - overlay.lex.start;

	if (wanted <= bytes_in_buffer)
		return;

	if (overlay.lex.eof) {
		err_type = SBC_ERROR_BAD_PROGRAM;
		return;
	}

	to_read = SBC_LEX_BUF_SIZE - bytes_in_buffer;

	/*
	 * src and destination cannot overlap as the buffer is 4 * the maximum
	 * token size.
	 */

	if ((bytes_in_buffer > 0) && (overlay.lex.start > 0))
		memcpy(overlay.lex.lex_buf,
		       &overlay.lex.lex_buf[overlay.lex.start],
		       bytes_in_buffer);
	read = specasm_file_read_e(overlay.lex.h,
				   &overlay.lex.lex_buf[bytes_in_buffer],
				   to_read);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SBC_ERROR_READ;
		return;
	}
	if (read < to_read)
		overlay.lex.eof = 1;
	overlay.lex.start = 0;
	overlay.lex.end = bytes_in_buffer + read;
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

	overlay.lex.start++;

	prv_ensure_buffer_e(1);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (overlay.lex.lex_buf[overlay.lex.start] == 0xff) {
		overlay.lex.tok.type = SBC_TOKEN_EOF;
		return;
	}

	prv_ensure_buffer_e(3);
	if (err_type != SPECASM_ERROR_OK)
		return;
	overlay.lex.tok.type = SBC_TOKEN_LINE_LABEL;
	overlay.lex.tok.tok.line_no =
		((uint16_t )overlay.lex.lex_buf[overlay.lex.start])
		<< 8;
	overlay.lex.start++;
	overlay.lex.tok.tok.line_no |=
		((uint16_t )overlay.lex.lex_buf[overlay.lex.start]);
	overlay.lex.start++;

	overlay.lex.line_no = overlay.lex.tok.tok.line_no;

	/*
	 * This 3rd byte contains the length of the line. This is handy
	 * as we can ensure this now and then now have to ensure until
	 * we get to another line.
	 */

	prv_ensure_buffer_e(overlay.lex.lex_buf[overlay.lex.start] - 1);
	overlay.lex.start++;
}

static void prv_handle_identifier(void)
{
	uint8_t ch;
	uint8_t len = 0;

	overlay.lex.tok.type = SBC_TOKEN_IDENTIFIER;

	do {
		len++;
		overlay.lex.start++;
		ch = overlay.lex.lex_buf[overlay.lex.start];
	} while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		 (ch >= 0 && ch <= 9) || (ch == '_'));

	if (ch == '$') {
		overlay.lex.tok.tok.id_type = SBC_ID_TYPE_STR;
		len++;
		overlay.lex.start++;
	} else if (ch == '%') {
		overlay.lex.tok.tok.id_type = SBC_ID_TYPE_INT;
		len++;
		overlay.lex.start++;
	} else {
		overlay.lex.tok.tok.id_type = SBC_ID_TYPE_REAL;
	}
	overlay.lex.tok.len = len;
	overlay.lex.tok.ptr = overlay.lex.start - len;
}

static void prv_handle_ext_keyword_e(void)
{
	uint8_t key;
	uint8_t first = overlay.lex.lex_buf[overlay.lex.start];

	overlay.lex.start++;
	key = overlay.lex.lex_buf[overlay.lex.start];

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

	overlay.lex.tok.type = SBC_TOKEN_KEYWORD;
	overlay.lex.tok.tok.keyword = key;
	overlay.lex.tok.ptr = overlay.lex.start;
	overlay.lex.tok.len = 1;
	overlay.lex.start++;
}

static void prv_handle_floating(int8_t f)
{
	float num;
	char *ptr;
	uint8_t bin_num[4];
	char *end_ptr = NULL;

	ptr = (char *) &overlay.lex.lex_buf[overlay.lex.start];

	/*
	 * This isn't ideal.  SDCC only supports floats and not doubles
	 * so we're going to lose some precision in our floating point
	 * constants as the Spectrum has 40 bit floats.  So we're just
	 * going to do a strtof and then convert the binary representation
	 * into one that is understood by the Spectrum.
	 */

	num = strtof(ptr, &end_ptr) * (float) f;

	overlay.lex.start += (end_ptr - ptr);
	memcpy(&bin_num, &num, sizeof(bin_num));

	/*
	 * Compute the exponent.  The bias is 128 instead of 127 so fix that
	 * up.
	 */

	overlay.lex.tok.tok.real.b[0] = bin_num[3] << 1;
	if (bin_num[2] & 0x80)
		overlay.lex.tok.tok.real.b[0]++;
	overlay.lex.tok.tok.real.b[0]++;

	/*
	 * Copy the mantissa.
	 */

	overlay.lex.tok.tok.real.b[1] = bin_num[2];
	overlay.lex.tok.tok.real.b[2] = bin_num[1];
	overlay.lex.tok.tok.real.b[3] = bin_num[0];
	overlay.lex.tok.tok.real.b[4] = 0;

	/*
	 * Copy the sign bit.
	 */

	overlay.lex.tok.tok.real.b[1] &= 0x7f;
	overlay.lex.tok.tok.real.b[1] |= bin_num[3] & 0x80;
	overlay.lex.tok.type = SBC_TOKEN_REAL;
}

static void prv_handle_decimal_e(int8_t f)
{
	const uint8_t *last;
	int32_t old_val;
	int32_t factor = f;
	int32_t val = 0;
	const uint8_t *start = &overlay.lex.lex_buf[overlay.lex.start];
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
	overlay.lex.tok.type = SBC_TOKEN_INTEGER;
	overlay.lex.tok.tok.integer = val;
	overlay.lex.start += (last - start);
}

static void prv_handle_complex_op(void)
{
	uint8_t second;
	const uint8_t *ptr;
	uint8_t first = overlay.lex.lex_buf[overlay.lex.start];

	overlay.lex.tok.type = SBC_TOKEN_OPERATOR;
	overlay.lex.tok.len = 1;
	overlay.lex.tok.ptr = overlay.lex.start;
	overlay.lex.start++;
	second = overlay.lex.lex_buf[overlay.lex.start];

	switch (first) {
	case '<':
		if ((second == '<') || (second == '>')  || (second == '='))
			break;
		return;
	case '>':
		if (second == '>') {
			if (overlay.lex.lex_buf[overlay.lex.start + 1] == '>') {
				overlay.lex.tok.len++;
				overlay.lex.start++;
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

		ptr = &overlay.lex.lex_buf[overlay.lex.start];
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		if (*ptr >= '0' && *ptr <= '9') {
			overlay.lex.start += ptr -
				&overlay.lex.lex_buf[overlay.lex.start];
			prv_handle_decimal_e(-1);
			return;
		} else if (*ptr == '.') {
			overlay.lex.start--;
			prv_handle_floating(1);
			return;
		}
		return;
	default:
		return;
	}
	overlay.lex.tok.len++;
	overlay.lex.start++;
}

static void prv_handle_string_e(void)
{
	const uint8_t *ptr;
	uint8_t len;
	const uint8_t *start = &overlay.lex.lex_buf[overlay.lex.start];

	for (ptr = start + 1; *ptr != '"'; ptr++) {
		if (*ptr == 13) {
			err_type = SBC_ERROR_MISSING_QUOTE;
			return;
		}
	}

	len = (ptr - start);
	overlay.lex.tok.type = SBC_TOKEN_STRING;
	overlay.lex.tok.len = len - 1;
	overlay.lex.tok.ptr = overlay.lex.start + 1;
	overlay.lex.start += len + 1;
}

static void prv_handle_line_no_e(void)
{
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t num[2];
	const uint8_t *ptr = &overlay.lex.lex_buf[overlay.lex.start + 1];

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

	memcpy(&overlay.lex.tok.tok.line_no, &num, 2);
	overlay.lex.tok.type = SBC_TOKEN_LINE_NUMBER;
	overlay.lex.start += 4;
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
	const uint8_t *ptr = &overlay.lex.lex_buf[overlay.lex.start + 1];
	uint8_t count = 1;

	dig = prv_is_hex_digit(*ptr);
	if (dig == -1) {
		err_type = SBC_ERROR_BAD_NUM;
		return;
	}

	overlay.lex.tok.tok.integer = dig;
	ptr++;
	while ((dig = prv_is_hex_digit(*ptr)) != -1) {
		if (count == 8) {
			err_type = SBC_ERROR_BAD_NUM;
			return;
		}
		overlay.lex.tok.tok.integer <<= 4;
		overlay.lex.tok.tok.integer |= dig;
		count++;
		ptr++;
	}
	overlay.lex.tok.type = SBC_TOKEN_HEX;
	overlay.lex.start += count + 1;
}

static void prv_handle_bin_e(void)
{
	const uint8_t *ptr = &overlay.lex.lex_buf[overlay.lex.start + 1];
	uint8_t count = 1;

	if (*ptr != '0' && *ptr != '1') {
		err_type = SBC_ERROR_BAD_NUM;
		return;
	}

	overlay.lex.tok.tok.integer = *ptr - '0';
	ptr++;
	while (*ptr == '0' || *ptr == '1') {
		if (count == 32) {
			err_type = SBC_ERROR_BAD_NUM;
			return;
		}
		overlay.lex.tok.tok.integer <<= 1;
		overlay.lex.tok.tok.integer |= *ptr - '0';
		count++;
		ptr++;
	}
	overlay.lex.tok.type = SBC_TOKEN_BIN;
	overlay.lex.start += count + 1;
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

	first = overlay.lex.lex_buf[overlay.lex.start];

	if (first == 13) {
		prv_handle_line_start_e();
		return;
	}

	while (first == ' ' || first == '\t') {
		overlay.lex.start++;
		first = overlay.lex.lex_buf[overlay.lex.start];
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
		overlay.lex.tok.type = SBC_TOKEN_OPERATOR;
		overlay.lex.tok.len = 1;
		overlay.lex.tok.ptr = overlay.lex.start;
		overlay.lex.start++;
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
		overlay.lex.tok.tok.keyword = first - 127;
		overlay.lex.tok.ptr = overlay.lex.start;
		overlay.lex.start++;
		if (overlay.lex.tok.tok.keyword == SBC_KEYWORD_REM) {
			overlay.lex.tok.type = SBC_TOKEN_REM;
			while (overlay.lex.lex_buf[overlay.lex.start] != 13)
				overlay.lex.start++;
			overlay.lex.tok.len = overlay.lex.start -
				overlay.lex.tok.ptr;
		} else {
			overlay.lex.tok.type = SBC_TOKEN_KEYWORD;
			overlay.lex.tok.len = 1;
		}
		return;
	}

	overlay.lex.tok.type = SBC_TOKEN_EOF;
}
