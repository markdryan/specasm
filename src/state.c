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

#include <string.h>

#include "state.h"

char scratch[SPECASM_MAX_SCRATCH];

uint8_t specasm_state_add_short_e(const char *str)
{
	uint8_t i;
	uint8_t max_strs = state.short_strs.num_strings;
	char *ptr = state.short_strs.strs;

	for (i = 0; i < max_strs && strcmp(ptr, str); i++)
		ptr += SPECASM_MAX_SHORT_LEN;

	if (i < max_strs)
		return i;

	if (max_strs == SPECASM_MAX_SHORT_STRINGS) {
		err_type = SPECASM_ERROR_TOO_MANY_SHORT_STRINGS;
		return 0xff;
	}

	strcpy(ptr, str);
	state.short_strs.num_strings++;

	return max_strs;
}

uint8_t specasm_state_add_long_e(const char *str)
{
	uint8_t i;
	uint8_t max_strs = state.long_strs.num_strings;
	char *ptr = state.long_strs.strs;

	for (i = 0; i < max_strs && strcmp(ptr, str); i++)
		ptr += SPECASM_MAX_LONG_LEN;

	if (i < max_strs)
		return i;

	if (max_strs == SPECASM_MAX_LONG_STRINGS) {
		err_type = SPECASM_ERROR_TOO_MANY_LONG_STRINGS;
		return 0xff;
	}

	strcpy(ptr, str);
	state.long_strs.num_strings++;

	return max_strs;
}

static uint8_t prv_is_reg(const char *str)
{
	uint8_t i;

	/* clang-format off */

	static const char* regs[] = {
		"a", "b", "c", "d", "e", "h", "l", "i", "r",
		"bc", "de", "hl", "af", "sp", "ix", "iy", "af'"
	};

	/* clang-format on */

	for (i = 0; i < sizeof(regs) / sizeof(*regs); i++)
		if (!strcmp(str, regs[i]))
			return 1;
	return 0;
}

void specasm_state_check_label_e(const char *str)
{
	const char *start = str;
	char ch = *str;

	if (ch != '_' &&
	    !((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) {
		err_type = SPECASM_ERROR_BAD_LABEL;
		return;
	}

	ch = *str++;
	while (ch) {
		if (ch != '_' &&
		    !((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
		      (ch >= '0' && ch <= '9'))) {
			err_type = SPECASM_ERROR_BAD_LABEL;
			return;
		}
		ch = *str++;
	}

	if (prv_is_reg(start))
		err_type = SPECASM_ERROR_BAD_LABEL;
}

static void prv_parse_label_e(const char *str, uint8_t i, specasm_line_t *line)
{
	uint8_t j;

	j = 0;
	for (; i < SPECASM_LINE_MAX_LEN && str[i] > ' ' &&
	       j < SPECASM_MAX_LONG_LEN - 1;
	     i++, j++)
		scratch[j] = str[i];
	if (j > SPECASM_MAX_LONG_LEN - 1) {
		err_type = SPECASM_ERROR_STRING_TOO_LONG;
		return;
	}
	scratch[j] = 0;
	specasm_state_check_label_e(scratch);
	if (err_type != SPECASM_ERROR_OK)
		return;
	if (j < SPECASM_MAX_SHORT_LEN) {
		line->data.label = specasm_state_add_short_e(scratch);
		if (err_type != SPECASM_ERROR_OK)
			return;
		line->type = SPECASM_LINE_TYPE_SL;
		return;
	}

	line->data.label = specasm_state_add_long_e(scratch);
	if (err_type != SPECASM_ERROR_OK)
		return;
	line->type = SPECASM_LINE_TYPE_LL;
	for (; i < SPECASM_LINE_MAX_LEN && str[i] == ' '; i++)
		;
	if (i < SPECASM_LINE_MAX_LEN)
		err_type = SPECASM_ERROR_LONG_LABEL_EX;
}

static void prv_parse_short_comment_e(const char *str, uint8_t i,
				      specasm_line_t *line)
{
	uint8_t limit = SPECASM_LINE_MAX_LEN - i;
	int8_t it;

	if (limit == 0)
		return;

	memcpy(scratch, &str[i], limit);
	scratch[limit] = 0;
	for (it = (int8_t)limit - 1; it >= 0 && scratch[it] == ' '; --it)
		;
	if (it < 0)
		return;
	it++;
	if ((uint8_t)it > SPECASM_MAX_SHORT_LEN - 1) {
		err_type = SPECASM_ERROR_BAD_COMMENT;
		return;
	}
	scratch[it] = 0;
	line->comment = specasm_state_add_short_e(scratch);
}

static void prv_parse_long_comment_e(const char *str, uint8_t i,
				     specasm_line_t *line)
{
	uint8_t j;
	uint8_t k;

	if (i == SPECASM_LINE_MAX_LEN)
		return;

	for (k = SPECASM_LINE_MAX_LEN - 1; k >= i && str[k] == ' '; k--)
		;
	if (k >= i) {
		j = (k - i) + 1;
		memcpy(scratch, &str[i], j + 1);
	} else {
		j = 0;
	}
	scratch[j] = 0;

	if (j < SPECASM_MAX_SHORT_LEN) {
		line->comment = specasm_state_add_short_e(scratch);
		line->type = SPECASM_LINE_TYPE_SC;
	} else {
		line->comment = specasm_state_add_long_e(scratch);
		line->type = SPECASM_LINE_TYPE_LC;
	}
	line->data.bad_comment = 0;
}

static uint8_t prv_parse_string_e(const char *str, char type, uint8_t i,
				  specasm_line_t *line)
{
	uint8_t k;
	uint8_t l;

	if (i == SPECASM_LINE_MAX_LEN)
		return i;

	switch (type) {
	case '\'':
		line->type = SPECASM_LINE_TYPE_STR_SIN_SHORT;
		break;
	case '"':
		line->type = SPECASM_LINE_TYPE_STR_DBL_SHORT;
		break;
	case '#':
		line->type = SPECASM_LINE_TYPE_STR_HSH_SHORT;
		break;
	case '@':
		line->type = SPECASM_LINE_TYPE_STR_AMP_SHORT;
		break;
	}

	for (k = i; k < SPECASM_LINE_MAX_LEN && str[k] != type; k++)
		;
	l = k + 1;
	k -= i;
	memcpy(scratch, &str[i], k);
	scratch[k] = 0;
	if (k < SPECASM_MAX_SHORT_LEN) {
		line->data.label = specasm_state_add_short_e(scratch);
	} else {
		line->data.label = specasm_state_add_long_e(scratch);
		line->type++;
	}

	return l;
}

static uint8_t prv_parse_include_e(const char *str, char type, uint8_t i,
				   specasm_line_t *line)
{
	uint8_t k;
	uint8_t l;

	for (; i < SPECASM_LINE_MAX_LEN && str[i] <= ' '; i++)
		;

	if (i == SPECASM_LINE_MAX_LEN)
		return i;

	for (k = SPECASM_LINE_MAX_LEN - 1; k > i && str[k] <= ' '; k--)
		;

	l = (k - i) + 1;
	line->type = (type == '-') ? SPECASM_LINE_TYPE_INC_SHORT
				   : SPECASM_LINE_TYPE_INC_SYS_SHORT;

	memcpy(scratch, &str[i], l);
	scratch[l] = 0;
	if (l < SPECASM_MAX_SHORT_LEN) {
		line->data.label = specasm_state_add_short_e(scratch);
	} else {
		line->data.label = specasm_state_add_long_e(scratch);
		line->type++;
	}

	return k + 1;
}

void specasm_set_comment(unsigned int l, const char *str)
{
	uint8_t i;
	specasm_line_t *line = &state.lines.lines[l];

	line->comment = SPECASM_NULL;

	// Skip white

	for (i = 0; i < SPECASM_LINE_MAX_LEN && str[i] <= 32; i++)
		;
	if (i == SPECASM_LINE_MAX_LEN) {
		line->type = SPECASM_LINE_TYPE_EMPTY;
		return;
	}

	prv_parse_long_comment_e(str, i, line);
	if (err_type != SPECASM_ERROR_OK) {
		line->type = SPECASM_LINE_TYPE_EMPTY;
		err_type = SPECASM_ERROR_OK;
	} else {
		line->data.bad_comment = 1;
	}
}

void specasm_append_empty_line_e(void)
{
	specasm_line_t *line;

	if (state.lines.num_lines == SPECASM_MAX_LINES - 1) {
		err_type = SPECASM_ERROR_TOO_MANY_LINES;
		return;
	}

	line = &state.lines.lines[state.lines.num_lines++];
	line->type = SPECASM_LINE_TYPE_EMPTY;
}

void specasm_delete_lines(unsigned int start, unsigned int end)
{
	if ((start >= end) || (end > state.lines.num_lines))
		return;

	for (; end < state.lines.num_lines; end++, start++)
		state.lines.lines[start] = state.lines.lines[end];
	state.lines.num_lines -= (end - start);
}

void specasm_insert_lines_e(unsigned int l, unsigned int count)
{
	specasm_line_t *line;
	unsigned int i;
	unsigned int old_last_line;
	unsigned int old_start_line;

	if (state.lines.num_lines + count >= SPECASM_MAX_LINES) {
		err_type = SPECASM_ERROR_TOO_MANY_LINES;
		return;
	}

	if (l >= state.lines.num_lines) {
		for (i = 0; i < count; i++)
			specasm_append_empty_line_e();
		return;
	}

	old_last_line = state.lines.num_lines - 1;
	old_start_line = l + count;

	for (i = old_last_line + count; i >= old_start_line; i--)
		state.lines.lines[i] = state.lines.lines[old_last_line--];

	for (; l < old_start_line; l++) {
		line = &state.lines.lines[i];
		line->type = SPECASM_LINE_TYPE_EMPTY;
	}

	state.lines.num_lines += count;
}

void specasm_parse_line_e(unsigned int l, const char *str)
{
	uint8_t i;
	specasm_line_t *line = &state.lines.lines[l];

	line->type = SPECASM_LINE_TYPE_EMPTY;
	line->comment = SPECASM_NULL;
	line->flags = 0;

	// Skip white

	for (i = 0; i < SPECASM_LINE_MAX_LEN && str[i] <= 32; i++)
		;
	if (i == SPECASM_LINE_MAX_LEN) {
		line->type = SPECASM_LINE_TYPE_EMPTY;
		return;
	}

	// We have a label

	if (str[i] == '.') {
		prv_parse_label_e(str, i + 1, line);
		return;
	}

	if (str[i] == ';') {
		prv_parse_long_comment_e(str, i + 1, line);
		return;
	}

	if (str[i] == '"' || str[i] == '\'' || str[i] == '@' || str[i] == '#')
		i = prv_parse_string_e(str, str[i], i + 1, line);
	else if (str[i] == '+' || str[i] == '-')
		i = prv_parse_include_e(str, str[i], i + 1, line);
	else
		i = specasm_parse_mnemomic_e(str, i, line);
	if (err_type != SPECASM_ERROR_OK)
		return;

	for (; i < SPECASM_LINE_MAX_LEN && str[i] <= 32; i++)
		;
	if (i >= SPECASM_LINE_MAX_LEN)
		return;

	if (str[i] != ';') {
		err_type = SPECASM_ERROR_BAD_COMMENT;
		return;
	}

	prv_parse_short_comment_e(str, i + 1, line);
}

static char *prv_format_string_e(char *buf, uint8_t lng, unsigned int l,
				 uint8_t ch)
{
	const char *ptr;

	if (lng)
		ptr = specasm_state_get_long_e(l);
	else
		ptr = specasm_state_get_short_e(l);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	*buf++ = ch;
	while (*ptr)
		*buf++ = *ptr++;

	return buf;
}

void specasm_format_line_e(char *buf, unsigned int l)
{
	const char *ptr;
	const char *end_ptr;
	char *comment_start;
	char *start;
	uint8_t i;
	const char str_ids[] = {'\'', '"', '#', '@', '-', '+'};
	const specasm_line_t *line = &state.lines.lines[l];

	end_ptr = buf + SPECASM_LINE_MAX_LEN;

	if (line->type == SPECASM_LINE_TYPE_EMPTY)
		goto clear;

	start = buf;
	if ((line->type == SPECASM_LINE_TYPE_LL) ||
	    (line->type == SPECASM_LINE_TYPE_SL)) {
		buf =
		    prv_format_string_e(buf, line->type == SPECASM_LINE_TYPE_LL,
					line->data.label, '.');
		goto clear;
	}

	if ((line->type == SPECASM_LINE_TYPE_LC) ||
	    (line->type == SPECASM_LINE_TYPE_SC)) {
		buf =
		    prv_format_string_e(buf, line->type == SPECASM_LINE_TYPE_LC,
					line->comment, ';');
		goto clear;
	}
	if ((line->type >= SPECASM_LINE_TYPE_STR_SIN_SHORT) &&
	    (line->type <= SPECASM_LINE_TYPE_INC_SYS_LONG)) {
		i = (line->type - SPECASM_LINE_TYPE_STR_SIN_SHORT) >> 1;
		i = str_ids[i];
		buf = prv_format_string_e(buf, line->type & 1, line->data.label,
					  i);
		if ((line->type < SPECASM_LINE_TYPE_INC_SHORT) && buf < end_ptr)
			*buf++ = i;
	} else {
		if (!((line->type >= SPECASM_LINE_TYPE_DB &&
		       line->type <= SPECASM_LINE_TYPE_DW_SUB) ||
		      line->type == SPECASM_LINE_TYPE_DS))
			for (i = 0; i < SPECASM_MAX_INDENT; i++)
				*buf++ = ' ';

		buf += specasm_dump_opcode_e(line, buf);
		if (err_type != SPECASM_ERROR_OK)
			goto clear;
	}

	if (line->comment != SPECASM_NULL) {
		comment_start = start + SPECASM_LINE_MAX_OPCODE + 1;
		while (buf < comment_start)
			*buf++ = ' ';
		*buf++ = ';';
		ptr = specasm_state_get_short_e(line->comment);
		if (err_type != SPECASM_ERROR_OK)
			goto clear;
		while (*ptr)
			*buf++ = *ptr++;
	}

clear:
	while (buf < end_ptr)
		*buf++ = ' ';
	*buf = 0;
}
