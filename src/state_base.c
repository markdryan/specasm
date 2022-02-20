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

#include <stdlib.h>
#include <string.h>

#include "peer.h"
#include "state_base.h"

specasm_state_t state;

void specasm_state_reset(void)
{
	state.lines.num_lines = 0;
	state.short_strs.num_strings = 0;
	state.long_strs.num_strings = 0;
	state.version = SPECASM_VERSION;
}

const char *specasm_state_get_short_e(uint8_t i)
{
	uint16_t off;

	if (i >= state.short_strs.num_strings) {
		err_type = SPECASM_ERROR_ASSERT_BAD_STRING_ID;
		return NULL;
	}

	off = i;
	off = specasm_short_string_offset(off);
	return &state.short_strs.strs[off];
}

const char *specasm_state_get_long_e(uint8_t i)
{
	uint16_t off;

	if (i >= state.long_strs.num_strings) {
		err_type = SPECASM_ERROR_ASSERT_BAD_STRING_ID;
		return NULL;
	}

	off = i;
	off = specasm_short_long_offset(off);
	return &state.long_strs.strs[off];
}

/*
 * Code for the fletcher16 algorithm copied from
 * https://en.wikipedia.org/wiki/Fletcher%27s_checksum#cite_note-3
 * Modified to take a uint16_t as the length.  We also use
 * modulo 256 instead of 255.  It makes the checksum worse but
 * module 255 is too slow.
 */

static uint16_t prv_fletcher16(const uint8_t *data, uint16_t len)
{
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	const uint8_t *end_data = data + len;

	while (data != end_data) {
		sum1 = (sum1 + *data++) & 255;
		sum2 = (sum2 + sum1) & 255;
	}

	return (sum2 << 8) | sum1;
}

void specasm_save_e(const char *fname)
{
	uint16_t checksum;

	checksum = prv_fletcher16((const uint8_t *)&state, sizeof(state));

	specasm_peer_write_state_e(fname, checksum);
}

void specasm_load_e(const char *fname)
{
	uint16_t checksum;
	uint16_t old_checksum;

	old_checksum = specasm_peer_read_state_e(fname);
	if (err_type == SPECASM_ERROR_OPEN)
		return;

	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	if (state.lines.num_lines > SPECASM_MAX_LINES) {
		err_type = SPECASM_ERROR_CORRUPT;
		goto on_error;
	}

	checksum = prv_fletcher16((const uint8_t *)&state, sizeof(state));
	if (checksum != old_checksum) {
		err_type = SPECASM_ERROR_CORRUPT;
		return;
	}

	if (SPECASM_VERSION >= state.version) {
		state.version = SPECASM_VERSION;
		return;
	}

	err_type = SPECASM_ERROR_SPECASM_TOO_OLD;

on_error:
	specasm_state_reset();
}

uint16_t specasm_compute_line_size(specasm_line_t *line)
{
	uint16_t id;
	const char *str;
	uint8_t shrt;
	uint8_t size = 0;

	if (line->type <= SPECASM_LINE_TYPE_SIMPLE_MAX)
		return (uint16_t)specasm_line_get_size(line) + 1;

	if (line->type == SPECASM_LINE_TYPE_REPB)
		return *((uint16_t *)&line->data.op_code[1]);

	if ((line->type >= SPECASM_LINE_TYPE_STR_SIN_SHORT) &&
	    (line->type <= SPECASM_LINE_TYPE_STR_AMP_LONG)) {
		id = line->data.label;
		if (line->type & 1)
			str = specasm_state_get_long_e(id);
		else
			str = specasm_state_get_short_e(id);
		size = strlen(str);

		/* We need to reserve one byte for the length. */

		shrt = line->type & 0xFE;
		if ((shrt == SPECASM_LINE_TYPE_STR_AMP_SHORT) ||
		    (shrt == SPECASM_LINE_TYPE_STR_HSH_SHORT))
			size++;
		return (uint16_t)size;
	}

	return 0;
}
