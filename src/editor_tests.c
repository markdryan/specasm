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
#include <string.h>

#ifdef SPECASM_TARGET_NEXT_OPCODES
#include "clipboard.h"
#endif

#include "editor.h"
#include "editor_buffers.h"
#include "editor_test_content.h"
#include "state.h"

extern uint8_t row;
extern uint8_t command_col;
extern uint8_t ovr;
extern uint8_t editing;
extern uint8_t mode;
extern unsigned int select_start;
extern unsigned int select_end;

static int prv_check_state(const editor_test_t *t)
{
	if (line != t->state.line) {
		printf("[FAIL]\n\tmismatched lines, expected %d got %d\n",
		       t->state.line, line);
		return 1;
	}
	if (col != t->state.col) {
		printf("[FAIL]\n\tmismatched col, expected %d got %d\n",
		       t->state.col, col);
		return 1;
	}
	if (row != t->state.row) {
		printf("[FAIL]\n\tmismatched row, expected %d got %d\n",
		       t->state.row, row);
		return 1;
	}
	if (command_col != t->state.command_col) {
		printf("[FAIL]\n\tmismatched command col, expected %d got %d\n",
		       t->state.command_col, command_col);
		return 1;
	}
	if (ovr != t->state.ovr) {
		printf("[FAIL]\n\tmismatched ovr, expected %d got %d\n",
		       t->state.ovr, ovr);
		return 1;
	}
	if (editing != t->state.editing) {
		printf("[FAIL]\n\tmismatched editing, expected %d got %d\n",
		       t->state.editing, editing);
		return 1;
	}
	if (quitting != t->state.quitting) {
		printf("[FAIL]\n\tmismatched quitting, expected %d got %d\n",
		       t->state.quitting, quitting);
		return 1;
	}
	if (mode != t->state.mode) {
		printf("[FAIL]\n\tmismatched mode, expected %d got %d\n",
		       t->state.mode, mode);
		return 1;
	}
	if (select_start != t->state.select_start) {
		printf("[FAIL]\n\tmismatched select_start,expected %d got %d\n",
		       t->state.select_start, select_start);
		return 1;
	}
	if (select_end != t->state.select_end) {
		printf("[FAIL]\n\tmismatched select_end, expected %d got %d\n",
		       t->state.select_end, select_end);
		return 1;
	}

	return 0;
}

static int prv_run_test(const editor_test_t *t)
{
	specasm_editor_reset();
#ifdef SPECASM_TARGET_NEXT_OPCODES
	specasm_clip_reset();
#endif
	printf("%s: ", t->name);

	for (size_t i = 0; i < strlen(t->input); i++) {
		specasm_handle_key_press((uint8_t)t->input[i]);
	}

	if (t->num_lines != state.lines.num_lines) {
		printf("[FAIL]\n\t%d lines expected.  Got %d\n", t->num_lines,
		       state.lines.num_lines);
		return 1;
	}

	char expected[SPECASM_LINE_MAX_LEN + 1];
	char got[SPECASM_LINE_MAX_LEN + 1];

	if (!t->command_test) {
		size_t to_check = t->num_lines;
		if (to_check > SPECASM_MAX_ROWS)
			to_check = SPECASM_MAX_ROWS;
		size_t offset = 0;
		for (size_t i = 0; i < to_check; i++) {
			memcpy(expected, &t->screen[offset],
			       SPECASM_LINE_MAX_LEN);
			expected[SPECASM_LINE_MAX_LEN] = 0;
			memcpy(got, &peer_unit_screen[offset],
			       SPECASM_LINE_MAX_LEN);
			got[SPECASM_LINE_MAX_LEN] = 0;
			if (memcmp(expected, got, SPECASM_LINE_MAX_LEN)) {
				printf("[FAIL]\n\tLine %zu: \"%s\" vs \"%s\"\n",
				       i, expected, got);
				return 1;
			}
			offset += SPECASM_LINE_MAX_LEN;
		}
	} else {
		memcpy(expected, t->screen, SPECASM_LINE_MAX_LEN);
		expected[SPECASM_LINE_MAX_LEN] = 0;
		memcpy(
		    got,
		    &peer_unit_screen[SPECASM_LINE_MAX_LEN * SPECASM_MAX_ROWS],
		    SPECASM_LINE_MAX_LEN);
		got[SPECASM_LINE_MAX_LEN] = 0;
		if (memcmp(expected, got, SPECASM_LINE_MAX_LEN)) {
			printf("[FAIL]\n\tCommand: \"%s\" vs \"%s\"\n",
			       expected, got);
			return 1;
		}
	}

	if (prv_check_state(t))
		return 1;

	printf("[OK]\n");

	return 0;
}

int run_editor_tests(void)
{
	for (size_t i = 0; i < editor_tests_count; i++)
		if (prv_run_test(&editor_tests[i]))
			return 1;

	return 0;
}
