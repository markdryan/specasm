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

#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
#include <stdlib.h>
#include <string.h>

#include "analysis.h"
#include "clipboard.h"
#include "editor.h"
#include "editor_buffers.h"
#include "editor_extra.h"
#include "line.h"
#include "scratch.h"
#include "state.h"

static void prv_clip_copy_e(const char *command)
{
	unsigned int i;

	specasm_clip_reset();
	for (i = select_start; i < select_end; i++) {
		specasm_format_line_e(line_buf, i);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
		specasm_clip_add_line_e(line_buf);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	return;

on_error:

	/*
	 * We need to put line_buf back the way it was.
	 */

	memset(line_buf, ' ', SPECASM_LINE_MAX_LEN);
	line_buf[0] = '>';
	line_buf[2] = command[0];
	if (command[1])
		line_buf[3] = command[1];
}

void specasm_selecting_clip_copy_e(void)
{
	if (select_start >= select_end)
		return;
	prv_clip_copy_e("cc");
}

void specasm_garbage_collect_e(void)
{
	uint8_t old_ovr = ovr;

	select_start = 0;
	select_end = state.lines.num_lines;
	prv_clip_copy_e("gc");
	if (err_type != SPECASM_ERROR_OK)
		return;

	specasm_editor_reset_no_cls();
	ovr = 1;

	specasm_selecting_clip_paste_e();
	specasm_clip_reset();
	ovr = old_ovr;
}

uint8_t specasm_selecting_clip_cut_e(void)
{
	if (select_start >= select_end)
		return 0;
	prv_clip_copy_e("x");
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (!specasm_selecting_delete())
		return 0;

	specasm_draw_screen(line - row);
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	specasm_draw_status();

	return 1;
}

void specasm_selecting_clip_paste_e(void)
{
	uint16_t i;
	uint16_t ptr = 0;
	uint16_t line_count = specasm_clip_get_line_count();

	if (line_count == 0)
		return;

	(void)specasm_make_space_e(line_count);
	if (err_type != SPECASM_ERROR_OK)
		return;

	i = line;
	ptr = specasm_clip_get_line(ptr, line_buf);
	while (ptr) {
		specasm_parse_line_e(i, line_buf);

		/*
		 * This can't really error unless there's some memory
		 * corruption.  If we return an error here we'll need
		 * to reset line_buf.  As this cannot happen it's not
		 * worth the code to do it.  We'll clear the error here
		 * to prevent the corrupted line_buf from being used
		 * when the command prompt is redisplayed.
		 */

		err_type = SPECASM_ERROR_OK;
		ptr = specasm_clip_get_line(ptr, line_buf);
		i++;
	}
	specasm_draw_screen(line - row);
	if (select_start < select_end) {
		select_end = select_start = 0;
		specasm_draw_status();
	}
}

void specasm_selecting_cycles(void)
{
	uint16_t i;
	char *ptr;
	specasm_cycles_t cycles;
	uint16_t m_count[2] = {0};
	uint16_t t_count[2] = {0};

	if (select_start >= select_end)
		return;

	for (i = select_start; i < select_end; i++) {
		specasm_get_cycles(&state.lines.lines[i], &cycles);
		m_count[0] += (uint16_t) cycles.m[0];
		m_count[1] += (uint16_t) cycles.m[1];
		t_count[0] += (uint16_t) cycles.t[0];
		t_count[1] += (uint16_t) cycles.t[1];
	}
	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	(void)itoa(m_count[0], scratch, 10);
	ptr = &scratch[strlen(scratch)];
	*ptr++ = '-';
	(void)itoa(m_count[1], ptr, 10);
	ptr = &ptr[strlen(ptr)];
	ptr[0] = ' ';
	ptr[1] = 'M';
	ptr[2] = ' ';
	ptr += 3;
	(void)itoa(t_count[0], ptr, 10);
	ptr = &ptr[strlen(ptr)];
	*ptr++ = '-';
	(void)itoa(t_count[1], ptr, 10);
	ptr = &ptr[strlen(ptr)];
	ptr[0] = ' ';
	ptr[1] = 'T';

	specasm_text_print(scratch, 0, SPECASM_MAX_ROWS, SPECASM_CODE_COLOUR);
	specasm_sleep_ms(1500);
}

void specasm_selecting_flags(void)
{
	uint16_t i;
	int8_t j;
	char *ptr;
	char flag;
	char flag_names[] = "cnp.h.sz";
	uint8_t flags = 0;

	if (select_start >= select_end)
		return;

	for (i = select_start; i < select_end; i++)
		flags |= specasm_get_flags(&state.lines.lines[i]);
	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	ptr = scratch;
	for (j = 7; j >= 0; j--) {
		flag = ((1 << j) & flags) ? flag_names[j] : '.';
		*ptr++ = flag;
	}

	specasm_text_print(scratch, 0, SPECASM_MAX_ROWS, SPECASM_CODE_COLOUR);
	specasm_sleep_ms(1500);
}

#endif
