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

#include "editor.h"
#include "editor_buffers.h"
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
#include "editor_extra.h"
#endif
#include "line.h"
#include "line_parse_common.h"
#include "peer.h"
#include "scratch.h"
#include "state.h"


#include <stdlib.h>
#include <string.h>

#ifdef UNITTESTS
#define EDITOR_STATIC
#else
#define EDITOR_STATIC static
#endif

EDITOR_STATIC uint8_t command_col;
EDITOR_STATIC uint8_t editing;
EDITOR_STATIC uint8_t mode;
uint8_t ovr;
uint8_t row;
unsigned int select_start;
unsigned int select_end;

static void specasm_dump_line_e(unsigned int l, uint8_t r, uint8_t inv)
{
	uint8_t col;
	uint8_t code_col;
	uint8_t label_col;
	uint8_t com_col;
	uint8_t data_col;
	uint8_t equ_col;
	uint8_t type;
	const specasm_line_t *line;

	if (inv) {
		code_col = SPECASM_SELECT_COLOUR;
		label_col = SPECASM_SELECT_COLOUR;
		com_col = SPECASM_SELECT_COLOUR;
		data_col = SPECASM_SELECT_COLOUR;
		equ_col = SPECASM_SELECT_COLOUR;
	} else {
		code_col = SPECASM_CODE_COLOUR;
		label_col = SPECASM_LABEL_COLOUR;
		com_col = SPECASM_COMMENT_COLOUR;
		data_col = SPECASM_DATA_COLOUR;
		equ_col = SPECASM_EQU_COLOUR;
	}

	line = &state.lines.lines[l];
	type = specasm_line_get_adj_type(line);
	if (type == SPECASM_LINE_TYPE_EMPTY) {
		specasm_text_clear(0, r, SPECASM_LINE_MAX_LEN, code_col);
		return;
	}

	specasm_format_line_e(scratch, l);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (type == SPECASM_LINE_TYPE_EQU) {
		col = equ_col;
	} else if ((type == SPECASM_LINE_TYPE_LL) ||
		   (type == SPECASM_LINE_TYPE_SL)) {
		col = label_col;
	} else if ((type == SPECASM_LINE_TYPE_LC) ||
		   (type == SPECASM_LINE_TYPE_SC)) {
		if (line->data.bad_comment && !inv)
			col = SPECASM_ERROR_COLOUR;
		else
			col = com_col;
	} else if (((type >= SPECASM_LINE_TYPE_STR_SIN_SHORT) &&
		    (type <= SPECASM_LINE_TYPE_STR_AMP_LONG)) ||
		   ((type >= SPECASM_LINE_TYPE_DB) &&
		    (type <= SPECASM_LINE_TYPE_DW_SUB)) ||
		   (type == SPECASM_LINE_TYPE_DS)) {
		col = data_col;
	} else {
		col = code_col;
		if (line->comment != SPECASM_NULL)
			scratch[SPECASM_LINE_MAX_OPCODE + 1] = 0;
	}
	if ((line->comment == SPECASM_NULL) || (type == SPECASM_LINE_TYPE_LC) ||
	    (type == SPECASM_LINE_TYPE_SC)) {
		(void)specasm_text_print(scratch, 0, r, col);
		return;
	}
	scratch[SPECASM_LINE_MAX_OPCODE + 1] = 0;
	(void)specasm_text_print(scratch, 0, r, col);
	scratch[SPECASM_LINE_MAX_OPCODE + 1] = ';';
	(void)specasm_text_print(&scratch[SPECASM_LINE_MAX_OPCODE + 1],
				 SPECASM_LINE_MAX_OPCODE + 1, r, com_col);
}

static void prv_selecting_count(void);
static void prv_selecting_copy_e(void);
static void prv_selecting_move(void);
static void prv_goto(const char *num);
static void prv_find(const char *needle);

static void prv_num_to_char(char *buf, unsigned int num, uint8_t digits)
{
	do {
		digits--;
		buf[digits] = '0' + (num % 10);
		num /= 10;
	} while (digits > 0);
}

static void prv_draw_error(void)
{
	uint8_t i;
	uint8_t len = SPECASM_LINE_MAX_LEN;

	i = specasm_text_print(specasm_error_msg(err_type), 0, SPECASM_MAX_ROWS,
			       SPECASM_ERROR_COLOUR);
	len -= i;
	memset(scratch, ' ', len);
	scratch[len] = 0;
	specasm_text_print(scratch, i, SPECASM_MAX_ROWS, SPECASM_ERROR_COLOUR);
	err_type = SPECASM_ERROR_OK;
}

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
void specasm_draw_status_banked(void)
#else
void specasm_draw_status(void)
#endif
{
	memset(scratch, ' ', 32);
	scratch[32] = 0;

	if (mode == SPECASM_MODE_SELECT) {
		scratch[0] = 'S';
		scratch[1] = 'E';
		scratch[2] = 'L';
	} else {
		if (ovr) {
			scratch[0] = 'O';
			scratch[1] = 'V';
			scratch[2] = 'R';
		} else {
			scratch[0] = 'I';
			scratch[1] = 'N';
			scratch[2] = 'S';
		}
		if (select_start < select_end)
			scratch[3] = '*';
	}

	prv_num_to_char(&scratch[11], state.lines.num_lines, 4);
	scratch[15] = '/';
	prv_num_to_char(&scratch[16], SPECASM_MAX_LINES, 4);
	prv_num_to_char(&scratch[25], col, 2);
	scratch[27] = ':';
	prv_num_to_char(&scratch[28], line, 4);
	(void)specasm_text_print(scratch, 0, 23, SPECASM_STATUS_COLOUR);
}

void specasm_draw_screen(unsigned int i)
{
	unsigned int j;
	uint8_t inv;
	specasm_line_t *l;

	for (j = 0; i < state.lines.num_lines && j < SPECASM_MAX_ROWS;
	     i++, j++) {
		inv = mode == SPECASM_MODE_SELECT && i >= select_start &&
		      i < select_end;
		specasm_dump_line_e(i, j, inv);
		if (err_type != SPECASM_ERROR_OK) {
			prv_draw_error();
			return;
		}
	}

	/*
	 * If the last line is not a blank line, which normally it is
	 * but might not be if someone edited the last line and then
	 * pressed the up key, and someone deletes another line, we
	 * need to clear the row where the last line was to ensure it
	 * doesn't get duplicated when we delete.
	 */

	if ((j < SPECASM_MAX_ROWS) && (i == state.lines.num_lines)) {
		l = &state.lines.lines[i - 1];
		if (l->type != SPECASM_LINE_TYPE_EMPTY) {
			specasm_text_clear(0, j, SPECASM_LINE_MAX_LEN,
					   SPECASM_CODE_COLOUR);
		}
	}
}

static uint8_t prv_update_line()
{
	if (!editing)
		return 1;

	select_start = select_end = 0;

	/*
	 * This should always be zero, but things will go weird if
	 * it's not.
	 */

	line_buf[SPECASM_LINE_MAX_LEN] = 0;
	specasm_parse_line_e(line, line_buf);
	if (err_type != SPECASM_ERROR_OK) {
		prv_draw_error();
		return 0;
	}
	specasm_dump_line_e(line, row, 0);
	err_type = SPECASM_ERROR_OK;

	editing = 0;
	return 1;
}

static uint8_t prv_start_buf()
{
	unsigned int off;

	if (line == 0)
		return 0;

	if (!prv_update_line())
		return 0;

	specasm_text_set_flash(col, row, 0);
	off = line - row;
	line = 0;
	col = 0;
	row = 0;
	if (off > 0)
		specasm_draw_screen(line);

	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_end_buf()
{
	unsigned int last_line;
	uint8_t redraw;

	last_line = state.lines.num_lines - 1;

	if (line == last_line)
		return 0;

	if (!prv_update_line())
		return 0;

	specasm_text_set_flash(col, row, 0);
	col = 0;
	if (state.lines.num_lines < SPECASM_MAX_ROWS) {
		line = last_line;
		row = last_line;
	} else {
		redraw = line - row != state.lines.num_lines - SPECASM_MAX_ROWS;
		row = SPECASM_MAX_ROWS - 1;
		line = last_line;
		if (redraw)
			specasm_draw_screen(line - (SPECASM_MAX_ROWS - 1));
	}
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_page_up()
{
	unsigned int starting_line;

	if (line == row)
		return 0;

	if (!prv_update_line())
		return 0;

	specasm_text_set_flash(col, row, 0);
	starting_line = line - row;
	if (starting_line > SPECASM_MAX_ROWS)
		line = starting_line - SPECASM_MAX_ROWS;
	else
		line = 0;
	row = 0;
	specasm_draw_screen(line);
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_page_down()
{
	if (state.lines.num_lines <= line + SPECASM_MAX_ROWS)
		return 0;

	if (!prv_update_line())
		return 0;

	specasm_text_set_flash(col, row, 0);
	line += SPECASM_MAX_ROWS;
	if (line + SPECASM_MAX_ROWS > state.lines.num_lines)
		line = state.lines.num_lines - SPECASM_MAX_ROWS;
	row = 0;
	specasm_draw_screen(line);
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_move_up(void)
{
	if (line == 0)
		return 0;

	if (!prv_update_line())
		return 0;
	if (row == 0) {
		--line;
		specasm_draw_screen(line);
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	}
	specasm_text_set_flash(col, row, 0);
	row--;
	line--;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_move_left(void)
{
	uint8_t ret;

	if (col == 0) {
		ret = prv_move_up();
		if (ret == 0)
			return ret;
		specasm_text_set_flash(col, row, 0);
		col = SPECASM_LINE_MAX_LEN;
	}
	specasm_text_set_flash(col, row, 0);
	col--;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_linestart(void)
{
	if (col == 0)
		return 0;
	specasm_text_set_flash(col, row, 0);
	col = 0;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

/*
 * Returns the column position after the last character in
 * buf.  If the line is completly full SPECASM_LINE_MAX_LEN - 1
 * is returned.  If the line is empty, 0 is returned.
 */

static uint8_t prv_last_real_char(const char *buf)
{
	uint8_t i;

	for (i = SPECASM_LINE_MAX_LEN - 1; i > 0 && buf[i] == ' '; i--)
		;
	if (i == SPECASM_LINE_MAX_LEN - 1)
		return i;
	if (i > 0 || *buf != ' ')
		i++;
	return i;
}

static uint8_t prv_line_end_pos(uint8_t *ch)
{
	const char *buf;
	uint8_t i;

	if (!editing) {
		specasm_format_line_e(scratch, line);
		err_type = SPECASM_ERROR_OK;
		buf = scratch;
	} else {
		buf = line_buf;
	}

	i = prv_last_real_char(buf);
	if (ch)
		*ch = buf[i];
	return i;
}

static void prv_start_command(void)
{
	if (!prv_update_line())
		return;
	mode = SPECASM_MODE_COMMAND;
	memset(line_buf, ' ', SPECASM_LINE_MAX_LEN);
	line_buf[SPECASM_LINE_MAX_LEN] = 0;
	line_buf[0] = '>';
	(void)specasm_text_print(line_buf, 0, SPECASM_MAX_ROWS,
				 SPECASM_CODE_COLOUR);
	command_col = 2;
	specasm_text_set_flash(col, row, 0);
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, SPECASM_FLASH);
}

static void prv_process_command_char(uint8_t k)
{
	uint8_t i;

	if (!ovr) {
		if (line_buf[SPECASM_LINE_MAX_LEN - 1] != ' ')
			return;
		for (i = SPECASM_LINE_MAX_LEN - 2; i >= command_col; i--)
			line_buf[i + 1] = line_buf[i];
		(void)specasm_text_print(line_buf, 0, SPECASM_MAX_ROWS,
					 SPECASM_CODE_COLOUR);
	} else {
		if (command_col >= SPECASM_LINE_MAX_LEN)
			return;
	}
	line_buf[command_col] = k;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
	specasm_text_printch(k, command_col, SPECASM_MAX_ROWS,
			     SPECASM_CODE_COLOUR);
	if (command_col < SPECASM_LINE_MAX_LEN - 1)
		command_col++;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, SPECASM_FLASH);
}

static void prv_move_command_left(void)
{
	if (command_col <= 2)
		return;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
	command_col--;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, SPECASM_FLASH);
}

static void prv_move_command_right(void)
{
	uint8_t last_ch;

	last_ch = prv_last_real_char(line_buf);
	if (command_col >= last_ch)
		return;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
	command_col++;
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, SPECASM_FLASH);
}

static void prv_delete_command(void)
{
	uint8_t i;

	if (command_col <= 2)
		return;

	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
	command_col--;
	if (ovr) {
		line_buf[command_col] = ' ';
		specasm_text_printch(' ', command_col, SPECASM_MAX_ROWS,
				     SPECASM_CODE_COLOUR);
	} else {
		for (i = command_col; i < SPECASM_LINE_MAX_LEN - 1; i++)
			line_buf[i] = line_buf[i + 1];
		line_buf[SPECASM_LINE_MAX_LEN - 1] = ' ';
		(void)specasm_text_print(line_buf, 0, SPECASM_MAX_ROWS,
					 SPECASM_CODE_COLOUR);
	}
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, SPECASM_FLASH);
}

static void prv_exit_command_mode(uint8_t m)
{
	specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
	mode = m;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	specasm_draw_status();
}

static uint8_t prv_single_char_command_e(uint8_t ch)
{
	uint8_t reset = 0;

	switch (ch) {
	case 'n':
		reset = 1;
		current_fname[0] = 0;
		break;
	case 'q':
		quitting = 1;
		break;
	case 'd':
		if (!specasm_selecting_delete())
			break;

		specasm_draw_screen(line - row);
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		specasm_draw_status();
		break;
	case 'b':
		prv_selecting_count();
		break;
	case 'c':
		prv_selecting_copy_e();
		break;
	case 'm':
		prv_selecting_move();
		break;
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
	case 't':
		specasm_selecting_cycles();
		break;
#endif
	case 'a':
		select_start = 0;
		select_end = state.lines.num_lines;
		break;
	case 's':
		if (!current_fname[0]) {
			err_type = SPECASM_ERROR_BAD_FNAME;
			return 0;
		}
		specasm_save_e(current_fname);
		break;
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
	case 'x':
		(void)specasm_selecting_clip_cut_e();
		break;
	case 'v':
		specasm_selecting_clip_paste_e();
		break;
#endif
	default:
		err_type = SPECASM_ERROR_BAD_COMMAND;
		return 0;
	}

	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_exit_command_mode(SPECASM_MODE_EDITOR);

	return reset;
}

static char *prv_complete_filename_e(char *com, uint8_t len)
{
	char *com2;

	/*
	 * Skip paste "l" or "s"
	 */

	com++;
	len--;

	while (*com == ' ') {
		++com;
		len--;
	}

	/*
	 * Do we already have a valid extension?
	 */

	com2 = com + len;
	if ((len > 2) && ((*com2 == 't') || (*com2 == 'x'))) {
		com2--;
		if (*com2 == '.')
			return com;
	}

	while ((com2 >= com) && (*com2 != '.') && (*com2 != '/'))
		com2--;

	/*
	 * We've got another extension where there shouldn't be one.
	 */

	if (*com2 == '.') {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return NULL;
	}

	/*
	 * If we get here the last part of the file name does not contain an
	 * extension.  There must be an extension so we'll add the default
	 * one before loading, '.x'.
	 */

	if (len + 2 >= SPECASM_MAX_SCRATCH) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return NULL;
	}
	com2 = com + len;
	com2[1] = '.';
	com2[2] = 'x';
	com2[3] = 0;

	return com;
}

static uint8_t prv_long_command_e(char *com, uint8_t len)
{
	uint8_t reset = 0;

	if (!strcmp(com, "sel")) {
		prv_exit_command_mode(SPECASM_MODE_SELECT);
		select_start = select_end = line;
		return 0;
	} else if (!strcmp(com, "ver")) {
		specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
		specasm_text_print(SPECASM_VERSION_STR "    ", 0,
				   SPECASM_MAX_ROWS, SPECASM_CODE_COLOUR);
		specasm_sleep_ms(1000);
	} else if (com[0] == 'l' && com[1] == ' ') {
		com = prv_complete_filename_e(com, len);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		specasm_load_e(com);
		if ((err_type != SPECASM_ERROR_OK) &&
		    (err_type != SPECASM_ERROR_OPEN)) {
			reset = 1;
			current_fname[0] = 0;
		} else if (err_type != SPECASM_ERROR_OK) {
			return 0;
		} else {
			strcpy(current_fname, com);
			line = row = col = select_end = select_start = 0;
			specasm_cls(SPECASM_CODE_COLOUR |
				    SPECASM_LABEL_BACKGROUND);
			specasm_draw_screen(0);
		}
	} else if (com[0] == 's' && com[1] == ' ') {
		com = prv_complete_filename_e(com, len);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		specasm_save_e(com);
		com[len + 1] = 0;
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		strcpy(current_fname, com);
	} else if (com[0] == 'g' && com[1] == ' ') {
		prv_goto(&com[2]);
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
	} else if ((com[0] == 'g') && (com[1] == 'c') && !com[2]) {
		specasm_garbage_collect_e();
#endif
	} else if (com[0] == 'f' && com[1] == ' ') {
		prv_find(&com[2]);
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
	} else if ((com[0] == 'c') && (com[1] == 'c') && !com[2]) {
		specasm_selecting_clip_copy_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
	} else if ((com[0] == 'f') && (com[1] == 'l') && !com[2]) {
		specasm_selecting_flags();
#endif
	} else {
		err_type = SPECASM_ERROR_BAD_COMMAND;
		return 0;
	}

	prv_exit_command_mode(SPECASM_MODE_EDITOR);

	return reset;
}

static void prv_enter_command(void)
{
	uint8_t i;
	uint8_t reset;

	for (i = SPECASM_LINE_MAX_LEN - 1; i > 1 && line_buf[i] == ' '; i--)
		;
	if (i == 1) {
		prv_exit_command_mode(SPECASM_MODE_EDITOR);
		return;
	}

	if (i == 2) {
		reset = prv_single_char_command_e(line_buf[2]);
	} else {
		line_buf[i + 1] = 0;
		reset = prv_long_command_e(&line_buf[2], i - 2);
	}

	if (err_type != SPECASM_ERROR_OK) {
		specasm_text_set_flash(command_col, SPECASM_MAX_ROWS, 0);
		prv_draw_error();
		specasm_sleep_ms(1000);

		/*
		 * Some commands append data to the line buffer.  It's
		 * easiest just to wipe the extra characters here rather
		 * than undoing the damage after each command.
		 */

		for (i++; i < SPECASM_LINE_MAX_LEN; i++)
			line_buf[i] = ' ';
		(void)specasm_text_print(line_buf, 0, SPECASM_MAX_ROWS,
					 SPECASM_CODE_COLOUR);
		specasm_text_set_flash(command_col, SPECASM_MAX_ROWS,
				       SPECASM_FLASH);
	}

	if (reset) {
		specasm_editor_reset();
		specasm_draw_status();
		specasm_text_set_flash(0, 0, SPECASM_FLASH);
	}
}

static uint8_t prv_lineend(void)
{
	uint8_t new_col;

	new_col = prv_line_end_pos(NULL);
	if (col == new_col)
		return 0;
	specasm_text_set_flash(col, row, 0);
	col = new_col;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static void prv_start_updating(void)
{
	if (!editing) {
		specasm_format_line_e(line_buf, line);
		if (err_type != SPECASM_ERROR_OK)
			prv_draw_error();

		(void)specasm_text_print(line_buf, 0, row, SPECASM_CODE_COLOUR);
		editing = 1;
	}
}

static uint8_t prv_check_delete_line(void)
{
	uint8_t last_ch;

	if (state.lines.num_lines == 1)
		return 0;

	if (!editing) {
		if (state.lines.lines[line].type != SPECASM_LINE_TYPE_EMPTY)
			return 0;
	} else {
		last_ch = prv_last_real_char(line_buf);
		if (last_ch > 0)
			return 0;
	}

	specasm_text_set_flash(col, row, 0);
	specasm_delete_lines(line, line + 1);
	if (line > 0) {
		line--;
		if (row > 0)
			row--;
	}
	specasm_draw_screen(line - row);
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	editing = 0;

	return 1;
}

static uint8_t prv_goto_line(unsigned int target)
{
	unsigned int starting_line;

	if (target == line)
		return 0;

	starting_line = line - row;
	line = target;
	col = 0;

	/*
	 * Are we going to a line that's already on the screen?
	 */
	if ((line >= starting_line) &&
	    (line < starting_line + SPECASM_MAX_ROWS)) {
		row = line - starting_line;
		return 0;
	}

	if (line + SPECASM_MAX_ROWS <= state.lines.num_lines) {
		starting_line = line;
		row = 0;
	} else {
		starting_line = (state.lines.num_lines - SPECASM_MAX_ROWS);
		row = line - starting_line;
	}
	specasm_draw_screen(starting_line);

	return 1;
}

static void prv_goto(const char *num)
{
	uint8_t flags;
	long val;
	unsigned int target;

	(void)specasm_get_long_imm_e(num, &val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (val < 0)
		target = 0;
	else {
		target = (unsigned int)val;
		if ((val > 0xffff) || (target >= state.lines.num_lines))
			target = state.lines.num_lines - 1;
	}

	(void)prv_goto_line(target);
}

static void prv_find(const char *needle)
{
	unsigned int l;

	for (l = line; l < state.lines.num_lines; l++) {
		specasm_format_line_e(scratch, l);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (strstr(scratch, needle)) {
			(void)prv_goto_line(l);
			return;
		}
	}
}

static uint8_t prv_delete(void)
{
	uint8_t new_col;
	uint8_t i;
	uint8_t update = 0;
	unsigned int old_line;

	if (col == 0) {
		if (!ovr) {
			old_line = line;
			if (!prv_check_delete_line())
				return 0;
			if (old_line == 0)
				return 1;
		} else {
			if (line == 0)
				return 0;
			if (prv_move_up() == 0)
				return update;
		}
		if (state.lines.lines[line].type == SPECASM_LINE_TYPE_EMPTY)
			return 1;
		prv_start_updating();
		new_col = prv_last_real_char(line_buf);
	} else {
		new_col = col - 1;
		prv_start_updating();
		specasm_text_set_flash(col, row, 0);
	}

	if (ovr) {
		col = new_col;
		line_buf[col] = ' ';
		specasm_text_printch(' ', col, row, SPECASM_CODE_COLOUR);
	} else {
		for (i = new_col; i < SPECASM_LINE_MAX_LEN - 1; i++)
			line_buf[i] = line_buf[i + 1];
		line_buf[SPECASM_LINE_MAX_LEN - 1] = ' ';
		(void)specasm_text_print(line_buf, 0, row, SPECASM_CODE_COLOUR);
		col = new_col;
	}
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_enter_end(void)
{
	if (row == SPECASM_MAX_ROWS - 1) {
		++line;
		specasm_draw_screen(line - (SPECASM_MAX_ROWS - 1));
		col = 0;
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	}
	specasm_text_set_flash(col, row, 0);
	row++;
	line++;
	col = 0;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_enter(void)
{
	if (!prv_update_line())
		return 0;

	if (line == state.lines.num_lines - 1) {
		specasm_append_empty_line_e();
		if (err_type != SPECASM_ERROR_OK) {
			prv_draw_error();
			return 0;
		}
	}

	return prv_enter_end();
}

static uint8_t prv_enter_insert(void)
{
	if (!prv_update_line())
		return 0;

	if (line < select_end)
		select_start = select_end = 0;

	specasm_insert_lines_e(col == 0 ? line : line + 1, 1);
	if (err_type != SPECASM_ERROR_OK) {
		prv_draw_error();
		return 0;
	}

	specasm_draw_screen(line - row);
	return prv_enter_end();
}

static uint8_t prv_move_right(void)
{
	if (col == SPECASM_LINE_MAX_LEN - 1)
		return prv_enter();
	specasm_text_set_flash(col, row, 0);
	col++;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_move_down(void)
{
	if (line == state.lines.num_lines - 1)
		return 0;

	if (!prv_update_line())
		return 0;

	if (row == SPECASM_MAX_ROWS - 1) {
		++line;
		specasm_draw_screen(line - (SPECASM_MAX_ROWS - 1));
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	}
	specasm_text_set_flash(col, row, 0);
	row++;
	line++;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

static uint8_t prv_insert_space(void)
{
	int8_t i;
	uint8_t ch;
	uint8_t end;

	end = prv_line_end_pos(&ch);
	if ((end == SPECASM_LINE_MAX_LEN - 1) && (ch != ' ')) {
		err_type = SPECASM_ERROR_NO_ROOM_IN_LINE;
		prv_draw_error();
		return 0;
	}

	prv_start_updating();

	for (i = SPECASM_LINE_MAX_LEN - 2; i >= (int8_t)col; i--)
		line_buf[i + 1] = line_buf[i];

	(void)specasm_text_print(line_buf, 0, row, SPECASM_CODE_COLOUR);

	return 1;
}

static uint8_t prv_process_char(uint8_t k)
{
	if (!ovr) {
		if (!prv_insert_space())
			return 0;
	} else {
		prv_start_updating();
	}
	line_buf[col] = k;
	specasm_text_set_flash(col, row, 0);
	specasm_text_printch(k, col, row, SPECASM_CODE_COLOUR);
	if (col < SPECASM_LINE_MAX_LEN - 1) {
		col++;
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	} else if (!ovr) {
		return prv_enter_insert();
	} else {
		return prv_enter();
	}
}

static void prv_keypress(uint8_t k)
{
	uint8_t update = 0;

	switch (k) {
	case SPECASM_KEY_LEFT:
		update = prv_move_left();
		break;
	case SPECASM_KEY_RIGHT:
		update = prv_move_right();
		break;
	case SPECASM_KEY_DOWN:
		update = prv_move_down();
		break;
	case SPECASM_KEY_UP:
		update = prv_move_up();
		break;
	case SPECASM_KEY_DELETE:
		update = prv_delete();
		break;
	case SPECASM_KEY_ENTER:
		if (ovr)
			update = prv_enter();
		else
			update = prv_enter_insert();
		break;
	case SPECASM_KEY_INSERT:
		ovr = !ovr;
		update = 1;
		break;
	case SPECASM_KEY_LINE_START:
		update = prv_linestart();
		break;
	case SPECASM_KEY_COMMAND:
		prv_start_command();
		break;
	case SPECASM_KEY_LINE_END:
		update = prv_lineend();
		break;
	case SPECASM_KEY_BUF_START:
		update = prv_start_buf();
		break;
	case SPECASM_KEY_BUF_END:
		update = prv_end_buf();
		break;
	case SPECASM_KEY_PAGE_UP:
		update = prv_page_up();
		break;
	case SPECASM_KEY_PAGE_DOWN:
		update = prv_page_down();
		break;
	default:
		update = 1;
		if (k > 31 && k < 127)
			update = prv_process_char(k);
		else
			return;
	}
	if (update)
		specasm_draw_status();
}

static uint8_t prv_select_key_down(void)
{
	uint8_t update;

	if (line == state.lines.num_lines - 1) {
		if (select_end > line)
			return 0;
		select_end = line + 1;
		specasm_dump_line_e(line, row, 1);
		err_type = SPECASM_ERROR_OK;
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 0;
	}

	if (line == select_end) {
		update = 1;
		select_end++;
	} else {
		update = 0;
		select_start++;
	}

	if (row == SPECASM_MAX_ROWS - 1) {
		++line;
		specasm_draw_screen(line - (SPECASM_MAX_ROWS - 1));
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	}

	specasm_text_set_flash(col, row, 0);
	specasm_dump_line_e(line, row, update);
	err_type = SPECASM_ERROR_OK;
	row++;
	line++;
	specasm_text_set_flash(col, row, SPECASM_FLASH);

	return 1;
}

static uint8_t prv_select_key_up(void)
{
	uint8_t update;

	if (line == 0)
		return 0;

	if (select_end == state.lines.num_lines) {
		select_end--;
		specasm_dump_line_e(line, row, 0);
		err_type = SPECASM_ERROR_OK;
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 0;
	}
	if (line > select_start) {
		update = 0;
		select_end--;
	} else {
		update = 1;
		select_start--;
	}

	if (row == 0) {
		--line;
		specasm_draw_screen(line);
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		return 1;
	}
	specasm_text_set_flash(col, row, 0);
	row--;
	line--;
	specasm_dump_line_e(line, row, update);
	err_type = SPECASM_ERROR_OK;
	specasm_text_set_flash(col, row, SPECASM_FLASH);
	return 1;
}

uint8_t specasm_selecting_delete(void)
{
	unsigned int screen_start;
	unsigned int range;

	if (select_start >= select_end)
		return 0;
	range = select_end - select_start;
	screen_start = line - row;

	specasm_delete_lines(select_start, select_end);

	/* Everything has been deleted */

	if (state.lines.num_lines == 0) {
		specasm_editor_reset();
		return 1;
	}

	/* start of selected area not on screen */

	if (select_start < screen_start) {
		row = 0;
		line = select_end - range;

		/* TODO: Is this condition possible. */
		if (line > state.lines.num_lines - 1)
			line = state.lines.num_lines - 1;
	}

	/*
	 * All or maybe just the first part of the selection
	 * is on the screen.
	 */

	else {
		line = select_start;
		if (line < SPECASM_MAX_ROWS)
			row = line;
		else
			row = line - screen_start;

		/* TODO: Is this condition possible. */
		if (select_start >= state.lines.num_lines) {
			line--;
			row--;
		}
	}
	select_end = select_start = 0;
	specasm_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	return 1;
}

uint16_t specasm_make_space_e(uint16_t line_count)
{
	uint16_t size;
	uint16_t lines_left;

	size = line_count;
	if (ovr) {
		lines_left = state.lines.num_lines - line;
		if (lines_left > size)
			lines_left = size;
		size -= lines_left;
	}

	if (size > 0) {
		specasm_insert_lines_e(line, size);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
	}

	return size;
}

static void prv_selecting_count(void)
{
	uint16_t i;
	char *ptr;
	uint16_t count = 0;

	if (select_start >= select_end)
		return;

	for (i = select_start; i < select_end; i++)
		count += specasm_compute_line_size(&state.lines.lines[i]);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	(void)itoa(count, scratch, 10);
	ptr = &scratch[strlen(scratch)];
	ptr[0] = ' ';
	ptr[1] = 'b';
	ptr[2] = 'y';
	ptr[3] = 't';
	ptr[4] = 'e';
	ptr[5] = 's';
	specasm_text_print(scratch, 0, SPECASM_MAX_ROWS, SPECASM_CODE_COLOUR);
	specasm_sleep_ms(1000);
}

static void prv_selecting_copy_e(void)
{
	uint16_t i;
	uint16_t j;
	uint16_t src_index;
	uint16_t size;
	specasm_line_t *src;
	specasm_line_t *dst;

	if (select_start >= select_end)
		return;

	size = specasm_make_space_e(select_end - select_start);
	if (err_type != SPECASM_ERROR_OK)
		return;

	j = line;
	for (i = select_start; i < select_end; i++) {
		if (i < line)
			src_index = i;
		else {
			src_index = i + size;
		}
		src = &state.lines.lines[src_index];
		dst = &state.lines.lines[j];
		*dst = *src;
		j++;
	}

	select_start = select_end = 0;
	specasm_draw_screen(line - row);
}

static void prv_selecting_move(void)
{
	uint16_t size;
	uint16_t dst_line;
	uint16_t src_line;
	specasm_line_t *src;
	specasm_line_t *dst;
	uint16_t i;
	uint8_t before;

	if (select_start >= select_end)
		return;

	/*
	 * Nothing to do, as we're moving the code to its
	 * current location.
	 */

	if (line >= select_start && line <= select_end) {
		select_end = select_start = 0;
		return;
	}

	/* We need at least 1 free line to do a move. */

	if (state.lines.num_lines == SPECASM_MAX_LINES) {
		err_type = SPECASM_ERROR_TOO_MANY_LINES;
		return;
	}

	size = select_end - select_start;

	dst_line = line;
	src_line = select_start;

	before = line < select_start;

	for (i = 0; i < size; i++) {
		/*
		 * We can ignore the error as we've already
		 * checked we have enough space.
		 */

		specasm_insert_lines_e(dst_line, 1);
		dst = &state.lines.lines[dst_line];
		if (before)
			src_line++;
		src = &state.lines.lines[src_line];
		*dst = *src;
		specasm_delete_lines(src_line, src_line + 1);
		if (before)
			dst_line++;
	}

	select_start = select_end = 0;
	specasm_draw_screen(line - row);
}

static void prv_selecting_keypress(uint8_t k)
{
	uint8_t update = 0;

	switch (k) {
	case SPECASM_KEY_DELETE:
#if defined(SPECASM_TARGET_NEXT_OPCODES) || defined(SPECASM_TARGET_128)
	case 'x':
		if (k == SPECASM_KEY_DELETE) {
			update = specasm_selecting_delete();
		} else {
			update = specasm_selecting_clip_cut_e();
			if (err_type != SPECASM_ERROR_OK) {
				prv_draw_error();
				specasm_sleep_ms(1000);
				update = 1;
			}
		}
#else
		update = specasm_selecting_delete();
#endif
	case ' ':
		select_start = 0;
		select_end = 0;
	case SPECASM_KEY_ENTER:
		mode = SPECASM_MODE_EDITOR;
		update = 1;
		specasm_draw_screen(line - row);
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		break;
	case 'a':
		line = state.lines.num_lines - 1;
		if (line < SPECASM_MAX_ROWS)
			row = line;
		else
			row = SPECASM_MAX_ROWS - 1;
		col = 0;
		select_start = 0;
		select_end = state.lines.num_lines;
		specasm_draw_screen(line - row);
		specasm_text_set_flash(col, row, SPECASM_FLASH);
		break;
	case SPECASM_KEY_DOWN:
		update = prv_select_key_down();
		break;
	case SPECASM_KEY_UP:
		update = prv_select_key_up();
		break;
	default:
		break;
	}
	if (update)
		specasm_draw_status();
}

static void prv_command_keypress(uint8_t k)
{
	switch (k) {
	case SPECASM_KEY_DELETE:
		prv_delete_command();
		break;
	case SPECASM_KEY_ENTER:
		prv_enter_command();
		break;
	case SPECASM_KEY_INSERT:
		ovr = !ovr;
		break;
	case SPECASM_KEY_LEFT:
		prv_move_command_left();
		break;
	case SPECASM_KEY_RIGHT:
		prv_move_command_right();
		break;
	default:
		if (k > 31 && k < 127)
			prv_process_command_char(k);
		else
			return;
		break;
	}
}

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
void specasm_handle_key_press_banked(uint8_t k)
#else
void specasm_handle_key_press(uint8_t k)
#endif
{
	if (mode == SPECASM_MODE_SELECT)
		prv_selecting_keypress(k);
	else if (mode == SPECASM_MODE_COMMAND)
		prv_command_keypress(k);
	else
		prv_keypress(k);
}

#ifdef SPECASM_TARGET_NEXT
#if defined(SPECASM_NEXT_BANKED)
void specasm_editor_preload_banked(const char *fname)
#else
void specasm_editor_preload(const char *fname)
#endif
{
	char *completed_fname;
	size_t len = strlen(fname);

	if (len > MAX_FNAME) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		goto on_error;
	}

	/*
	 * We re-use prv_complete_filename_e here to avoid having to
	 * duplicate the code.  This function expects to be passed a
	 * buffer (which in effect is a pointer to the start of the
	 * line_buf buffer), the first character of which is 'l' which it skips.
	 * So we'll just duplicate this setup so we can re-use the code.
	 */

	line_buf[0] = ' ';
	strcpy(&line_buf[1], fname);
	completed_fname = prv_complete_filename_e(line_buf, len);

	specasm_load_e(completed_fname);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	strcpy(current_fname, completed_fname);
	line = row = col = select_end = select_start = 0;
	specasm_draw_screen(0);

	return;

on_error:
	prv_draw_error();
	specasm_sleep_ms(1000);
	err_type = SPECASM_ERROR_OK;
	specasm_editor_reset();
}
#endif

void specasm_editor_reset_no_cls(void)
{
	specasm_line_t *l;

	ovr = line = row = col = select_end = select_start = editing = 0;
	mode = quitting = command_col = 0;
	specasm_state_reset();
	l = &state.lines.lines[0];
	state.lines.num_lines = 1;
	l->type = SPECASM_LINE_TYPE_EMPTY;
	err_type = SPECASM_ERROR_OK;
}

#if defined(SPECASM_NEXT_BANKED) || defined(SPECASM_128_BANKED)
void specasm_editor_reset_banked(void)
#else
void specasm_editor_reset(void)
#endif
{
	specasm_editor_reset_no_cls();
	specasm_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
}
