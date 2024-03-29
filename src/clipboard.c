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

#ifdef SPECASM_TARGET_NEXT_OPCODES

#include <stdlib.h>
#include <string.h>

#include "clipboard.h"
#include "line.h"

#define SPECASM_CLIP_MAX_SIZE (15 * 512)

static uint8_t clip_buffer[SPECASM_CLIP_MAX_SIZE];

static uint16_t clip_end_ptr;
static uint16_t clip_lines;

#ifdef SPECASM_NEXT_BANKED
void specasm_clip_reset_banked(void)
#else
void specasm_clip_reset(void)
#endif
{
	clip_end_ptr = 0;
	clip_lines = 0;
}

#ifdef SPECASM_NEXT_BANKED
void specasm_clip_add_line_banked_e(const char *line)
#else
void specasm_clip_add_line_e(const char *line)
#endif
{
	uint8_t len;
	const char *end_ptr;

	/*
	 * Skip leading and trailing white space.  We don't need to store this.
	 */

	end_ptr = &line[SPECASM_LINE_MAX_LEN - 1];
	while ((end_ptr >= line) && (*end_ptr == ' '))
		end_ptr--;
	end_ptr++;

	if (line == end_ptr) {
		clip_buffer[clip_end_ptr++] = 0;
	} else {
		while (*line == ' ')
			line++;

		len = end_ptr - line;
		if (len + clip_end_ptr >= SPECASM_CLIP_MAX_SIZE) {
			err_type = SPECASM_ERROR_TOO_MANY_LINES;
			return;
		}

		clip_buffer[clip_end_ptr++] = len;
		memcpy(&clip_buffer[clip_end_ptr], line, len);
		clip_end_ptr += len;
	}
	clip_lines++;
}

#ifdef SPECASM_NEXT_BANKED
uint16_t specasm_clip_get_line_banked(uint16_t ptr, char *buffer)
#else
uint16_t specasm_clip_get_line(uint16_t ptr, char *buffer)
#endif
{
	uint8_t len;
	uint8_t spaces;

	if (ptr >= clip_end_ptr)
		return 0;

	len = clip_buffer[ptr++];
	if (len > 0) {
		memcpy(buffer, &clip_buffer[ptr], len);
		ptr += len;
	}

	spaces = SPECASM_LINE_MAX_LEN - len;
	if (spaces > 0)
		memset(&buffer[len], ' ', spaces);
	buffer[len + spaces] = 0;

	return ptr;
}

#ifdef SPECASM_NEXT_BANKED
uint16_t specasm_clip_get_line_count_banked(void)
#else
uint16_t specasm_clip_get_line_count(void)
#endif
{
	return clip_lines;
}

#endif
