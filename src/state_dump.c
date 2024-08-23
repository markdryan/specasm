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

static char *prv_format_equ_e(char *buf, const specasm_line_t *line)
{
	const uint8_t *op_code = &line->data.op_code[0];
	buf = prv_format_string_e(buf, op_code[0] == SPECASM_LINE_TYPE_LL,
				  op_code[1], '.');
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	buf[0] = ' ';
	buf[1] = 'e';
	buf[2] = 'q';
	buf[3] = 'u';

	return prv_format_string_e(buf + 4, op_code[2] == SPECASM_LINE_TYPE_LL,
				   op_code[3], ' ');
}

/*
 * buf must be SPECASM_MAX_SCRATCH or greater.  This is larger than
 * SPECASM_LINE_MAX_LEN.  The problem is since we added support for
 * expressions it's possible to enter a line in the editor, which when
 * formatted, will be > SPECASM_LINE_MAX_LEN.  Ideally, we'd detect
 * this when formatting, but this would introduce many if statements,
 * so it's easier to use a slightly larger buffer to overflow into and
 * then to check for the actual overflow once the formatting has finished,
 * which is what we now do.
 */

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
void specasm_format_line_banked_e(char *buf, unsigned int l)
#else
void specasm_format_line_e(char *buf, unsigned int l)
#endif
{
	const char *ptr;
	const char *end_ptr;
	char *comment_start;
	char *start;
	uint8_t i;
	const char str_ids[] = {'\'', '"', '#', '@', '-', '+', '!'};
	const specasm_line_t *line = &state.lines.lines[l];
	uint8_t type = specasm_line_get_adj_type(line);

	end_ptr = buf + SPECASM_LINE_MAX_LEN;

	if (type == SPECASM_LINE_TYPE_EMPTY)
		goto clear;

	start = buf;

	if (type == SPECASM_LINE_TYPE_EQU) {
		buf = prv_format_equ_e(buf, line);
		goto clear;
	}

	if ((type == SPECASM_LINE_TYPE_LL) || (type == SPECASM_LINE_TYPE_SL)) {
		buf = prv_format_string_e(buf, type == SPECASM_LINE_TYPE_LL,
					  line->data.label, '.');
		goto clear;
	}

	if ((type == SPECASM_LINE_TYPE_LC) || (type == SPECASM_LINE_TYPE_SC)) {
		buf = prv_format_string_e(buf, type == SPECASM_LINE_TYPE_LC,
					  line->comment, ';');
		goto clear;
	}
	if ((type >= SPECASM_LINE_TYPE_STR_SIN_SHORT) &&
	    (type <= SPECASM_LINE_TYPE_INC_BIN_LONG)) {
		i = (type - SPECASM_LINE_TYPE_STR_SIN_SHORT) >> 1;
		i = str_ids[i];
		buf = prv_format_string_e(buf, line->type & 1, line->data.label,
					  i);
		if ((type < SPECASM_LINE_TYPE_INC_SHORT) && buf < end_ptr)
			*buf++ = i;
	} else {
		if (!((type >= SPECASM_LINE_TYPE_DB &&
		       type <= SPECASM_LINE_TYPE_DW_SUB) ||
		      type == SPECASM_LINE_TYPE_DS))
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
	if (buf > end_ptr)
		err_type = SPECASM_ERROR_NO_ROOM_IN_LINE;

	while (buf < end_ptr)
		*buf++ = ' ';
	*buf = 0;
}
