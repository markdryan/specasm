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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "editor.h"
#include "state.h"

/* clang-format off */

#define EDITOR_BUF_START "\x7"
#define EDITOR_KEY_LEFT "\x8"
#define EDITOR_KEY_RIGHT "\x9"
#define EDITOR_BUF_END "\x6"
#define EDITOR_PAGE_DOWN "\x81"
#define EDITOR_PAGE_UP "\x80"
#define EDITOR_KEY_DOWN "\xa"
#define EDITOR_KEY_UP "\xb"
#define EDITOR_KEY_DELETE "\xc"
#define EDITOR_KEY_ENTER "\xd"
#define EDITOR_KEY_LINE_END "\x85"
#define EDITOR_KEY_INSERT "\x82"
#define EDITOR_KEY_LINE_START "\x83"
#define EDITOR_KEY_COMMAND "\x84"


#define FORMATTED_TEST_DATA \
	"; Here's an example hello the co"\
	"; program with some             "\
	"; comments                      "\
	";                               "\
	"                                "\
	".this_is_the_main_loop          "\
	"  halt              ; here      "\
	"  halt              ; next      "\
	"  halt                          "\
	"  scf                           "\
	".end                            "\
	"  rla                           "\
	";1                              "\
	";2                              "\
	";3                              "\
	".four                           "\
	".five                           "\
	".six                            "\
	"; seven                         "\
	"; 8                             "\
	";  nine                         "\
	";   ten                         "\
	";     11                        "\
	"; )-:                           "\
	"; (-:                           "\
	";-)                             "

const char *const formatted_test_data = FORMATTED_TEST_DATA
	"                                ";
const char *const formatted_test_data2x  = FORMATTED_TEST_DATA
	FORMATTED_TEST_DATA
	"                                ";

const char *const formatted_last_deleted =
	"                                "
	".this_is_the_main_loop          "
	"  halt              ; here      "
	"  halt              ; next      "
	"  halt                          "
	"  scf                           "
	".end                            "
	"  rla                           "
	";1                              "
	";2                              "
	";3                              "
	".four                           "
	".five                           "
	".six                            "
	"; seven                         "
	"; 8                             "
	";  nine                         "
	";   ten                         "
	";     11                        "
	"; )-:                           "
	"; (-:                           "
	"                                "
	"                                ";

#define test_data "; Here's an example hello the co"\
	" ; program with some" EDITOR_KEY_ENTER\
	" ; comments" EDITOR_KEY_ENTER\
	";" EDITOR_KEY_ENTER\
	"" EDITOR_KEY_ENTER\
	".this_is_the_main_loop" EDITOR_KEY_ENTER\
	"halt ; here" EDITOR_KEY_ENTER\
	"halt ; next" EDITOR_KEY_ENTER\
	"halt" EDITOR_KEY_ENTER\
	"scf" EDITOR_KEY_ENTER\
	".end" EDITOR_KEY_ENTER\
	"rla" EDITOR_KEY_ENTER\
	";1" EDITOR_KEY_ENTER\
	";2" EDITOR_KEY_ENTER\
	";3" EDITOR_KEY_ENTER\
	".four" EDITOR_KEY_ENTER\
	".five"EDITOR_KEY_ENTER\
	".six" EDITOR_KEY_ENTER\
	"; seven" EDITOR_KEY_ENTER\
	"; 8" EDITOR_KEY_ENTER\
	";  nine" EDITOR_KEY_ENTER\
	";   ten" EDITOR_KEY_ENTER\
	";     11" EDITOR_KEY_ENTER\
	"; )-:" EDITOR_KEY_ENTER\
	"; (-:" EDITOR_KEY_ENTER\
	";-)" EDITOR_KEY_ENTER

/* clang-format on */

extern uint8_t row;
extern uint8_t command_col;
extern uint8_t ovr;
extern uint8_t editing;
extern uint8_t mode;
extern unsigned int select_start;
extern unsigned int select_end;

struct editor_test_state_t_ {
	unsigned int line;
	uint8_t col;
	uint8_t row;
	uint8_t command_col;
	uint8_t ovr;
	uint8_t editing;
	uint8_t quitting;
	uint8_t mode;
	unsigned int select_start;
	unsigned int select_end;
};
typedef struct editor_test_state_t_ editor_test_state_t;

struct editor_test_t_ {
	const char *name;
	const char *input;
	const char *screen;
	const char *atts;
	editor_test_state_t state;
	unsigned int num_lines;
	unsigned int command_test;
};

typedef struct editor_test_t_ editor_test_t;

/* clang-format off */

#define EDITOR_BLANK_LINE "                                "


const editor_test_t editor_tests[] = {
	{
		"reset",
		"",
		EDITOR_BLANK_LINE,
		"",
		{ 0 },
		1,
	},
	{
		"add_empty",
		EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1, },
		2,
	},
	{
		"blank_line_0",
		EDITOR_KEY_ENTER "0",
		EDITOR_BLANK_LINE
		"0                               ",
		"",
		{ .line = 1, .row = 1, .col = 1, .editing = 1 },
		2,
	},
	{
		"two_empty_delete",
		EDITOR_KEY_ENTER EDITOR_KEY_ENTER EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1 },
		2,
	},
	{
		"delete_empty",
		EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE,
		"",
		{ 0 },
		1,
	},
	{
		"add_empty_ovr",
		EDITOR_KEY_INSERT EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1, .ovr = 1},
		2,
	},
	{
		"blank_line_0_ovr",
		EDITOR_KEY_INSERT EDITOR_KEY_ENTER "0",
		EDITOR_BLANK_LINE
		"0                               ",
		"",
		{ .line = 1, .row = 1, .col = 1, .editing = 1, .ovr = 1 },
		2,
	},
	{
		"two_empty_delete_ovr",
		EDITOR_KEY_INSERT EDITOR_KEY_ENTER EDITOR_KEY_ENTER
		EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE EDITOR_BLANK_LINE EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1, .ovr = 1 },
		3,
	},
	{
		"test_data",
		test_data,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22 },
		27,
	},
	{
		"test_new",
		test_data EDITOR_KEY_COMMAND "n" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .row = 0 },
		1,
	},
	{
		"test_up",
		test_data EDITOR_KEY_UP,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 25, .row = 21 },
		27,
	},
	{
		"scroll_up",
		test_data EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP,
		&formatted_test_data[3 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 3, .row = 0 },
		27,
	},
	{
		"page_up",
		test_data EDITOR_PAGE_UP,
		formatted_test_data,
		"",
		{ .line = 0, .row = 0 },
		27,
	},
	{
		"scroll_down",
		test_data EDITOR_PAGE_UP EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN,
		&formatted_test_data[1 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 23, .row = 22 },
		27,
	},
	{
		"page_up_page_up",
		test_data EDITOR_PAGE_UP EDITOR_PAGE_UP,
		formatted_test_data,
		"",
		{ .line = 0, .row = 0 },
		27,
	},
	{
		"page_down",
		test_data EDITOR_PAGE_DOWN,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22 },
		27,
	},
	{
		"up4_page_down",
		test_data EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_PAGE_DOWN,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 22, .row = 18 },
		27,
	},
	{
		"page_up_page_down",
		test_data EDITOR_PAGE_UP EDITOR_PAGE_DOWN,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 4, .row = 0 },
		27,
	},
	{
		"page_up_big",
		test_data test_data EDITOR_PAGE_UP,
		&formatted_test_data2x[7 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 7, .row = 0 },
		53,
	},
	{
		"page_down_big",
		test_data test_data test_data EDITOR_BUF_START EDITOR_PAGE_DOWN,
		&formatted_test_data2x[23 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 23, .row = 0 },
		79,
	},
	{
		"top",
		test_data test_data EDITOR_BUF_START,
		formatted_test_data2x,
		"",
		{ .line = 0, .row = 0 },
		53,
	},
	{
		"top_bottom",
		test_data test_data EDITOR_BUF_START EDITOR_BUF_END,
		&formatted_test_data2x[30 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 52, .row = 22 },
		53,
	},
	{
		"bottom_top_bottom",
		"ld a, 10" EDITOR_KEY_ENTER EDITOR_BUF_END EDITOR_BUF_START
		EDITOR_BUF_END,
		"  ld a, 10                      "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1 },
		2,
	},
	{
		"delete",
		"1234" EDITOR_KEY_DELETE EDITOR_KEY_DELETE EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .editing = 1 },
		1,
	},
	{
		"delete_ovr",
		"1234" EDITOR_KEY_INSERT EDITOR_KEY_DELETE EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE EDITOR_KEY_DELETE EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .editing = 1, .ovr = 1 },
		1,
	},
	{
		"delete_cmd_ovr",
		EDITOR_KEY_COMMAND "1234" EDITOR_KEY_INSERT EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE EDITOR_KEY_DELETE EDITOR_KEY_DELETE,
		">                               ",
		"",
		{ .line = 0, .row = 0, .ovr = 1, .command_col = 2,
		  .mode = SPECASM_MODE_COMMAND },
		1,
		1,
	},
	{
		"delete_ovr2",
		"1234" EDITOR_KEY_INSERT EDITOR_KEY_LEFT EDITOR_KEY_DELETE,
		"12 4                            ",
		"",
		{ .line = 0, .row = 0, .col = 2, .editing = 1, .ovr = 1 },
		1,
	},
	{
		"delete_cmd_ovr2",
		EDITOR_KEY_COMMAND "1234" EDITOR_KEY_INSERT EDITOR_KEY_LEFT
		EDITOR_KEY_DELETE,
		"> 12 4                          ",
		"",
		{ .line = 0, .row = 0, .command_col = 4, .ovr = 1,
		  .mode = SPECASM_MODE_COMMAND },
		1,
		1,
	},
	{
		"delete_all",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0 },
		1,
	},
	{
		"delete_nothing",
		test_data EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22, .command_col = 3 },
		27,
	},
	{
		"delete_almost_all",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_ENTER EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .command_col = 3 },
		1,
	},
	{
		"delete_almost_all_select",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .command_col = 5 },
		1,
	},
	{
		"delete_nothing_select",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DELETE,
		formatted_test_data,
		"",
		{ .line = 0, .row = 0, .command_col = 5 },
		27,
	},
	{
		"delete_all_select_up",
		test_data EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_ENTER EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .command_col = 3 },
		1,
	},
	{
		"delete_almost_all_select_up",
		test_data EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_ENTER EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		"; Here's an example hello the co"
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1, .command_col = 3 },
		2,
	},
	{
		"start_line",
		"123456" EDITOR_KEY_LINE_START,
		"123456                          ",
		"",
		{ .line = 0, .row = 0, .editing = 1 },
		1,
	},
	{
		"end_line",
		";12345" EDITOR_KEY_ENTER EDITOR_KEY_UP EDITOR_KEY_LINE_END,
		";12345                          "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 6 },
		2,
	},
	{
		"end_full_line",
		"; Here's an example hello the co"
		EDITOR_KEY_UP EDITOR_KEY_LINE_END,
		"; Here's an example hello the co"
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 31 },
		2,
	},
	{
		"insert_editing",
		"; Here's an example hello the c"
		EDITOR_KEY_LEFT EDITOR_KEY_LEFT " ",
		"; Here's an example hello the  c",
		"",
		{ .line = 0, .row = 0, .col = 30, .editing = 1 },
		1,
	},
	{
		"insert_full_editing",
		"; Here's an example hello the c"
		EDITOR_KEY_LEFT EDITOR_KEY_LEFT "  ",
		"; Here's an example hello the  c",
		"",
		{ .line = 0, .row = 0, .col = 30, .editing = 1 },
		1,
	},
	{
		"insert_not_editing",
		"; Here's an example hello the c"
		EDITOR_KEY_ENTER EDITOR_KEY_UP EDITOR_KEY_LEFT " ",
		" ; Here's an example hello the c"
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 1, .editing = 1 },
		2,
	},
	{
		"navigate",
		test_data
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_DOWN
		EDITOR_KEY_RIGHT EDITOR_KEY_RIGHT EDITOR_KEY_RIGHT
		EDITOR_KEY_DOWN,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 25, .row = 21, .col = 3 },
		27,
	},
	{
		"cant_go_down",
		EDITOR_KEY_DOWN,
		EDITOR_BLANK_LINE,
		"",
		{ 0 },
		1,
	},
	{
		"right_update",
		"; Here's an example hello the c"
		EDITOR_KEY_RIGHT,
		"; Here's an example hello the c "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1, .col = 0, .editing = 0 },
		2,
	},
	{
		"command_left",
		EDITOR_KEY_COMMAND "1234" EDITOR_KEY_LEFT EDITOR_KEY_LEFT,
		"> 1234                          ",
		"",
		{ .command_col = 4, .mode = SPECASM_MODE_COMMAND },
		1,
		1,
	},
	{
		"command_right",
		EDITOR_KEY_COMMAND "1234" EDITOR_KEY_LEFT EDITOR_KEY_LEFT
		EDITOR_KEY_RIGHT,
		"> 1234                          ",
		"",
		{ .command_col = 5, .mode = SPECASM_MODE_COMMAND},
		1,
		1,
	},
	{
		"command_noright",
		EDITOR_KEY_COMMAND "1234" EDITOR_KEY_RIGHT,
		"> 1234                          ",
		"",
		{ .command_col = 6, .mode = SPECASM_MODE_COMMAND},
		1,
		1,
	},
	{
		"quitting",
		EDITOR_KEY_COMMAND "q" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 3, .quitting =1 },
		1,
	},
	{
		"command_delete",
		EDITOR_KEY_COMMAND "qw" EDITOR_KEY_DELETE EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 3, .quitting =1 },
		1,
	},
	{
		"command_overwrite",
		EDITOR_KEY_COMMAND "set" EDITOR_KEY_INSERT EDITOR_KEY_LEFT "l"
		EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 5, .mode = SPECASM_MODE_SELECT, .ovr = 1 },
		1,
	},
	{
		"select_last_up",
		test_data EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_UP,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22, .command_col = 5,
		  .mode = SPECASM_MODE_SELECT, .select_start = 24,
		  .select_end = 26},
		27,
	},
	{
		"select_deselect",
		test_data EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_UP EDITOR_KEY_UP,
		&formatted_test_data[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 24, .row = 20, .command_col = 5,
		  .mode = SPECASM_MODE_SELECT, .select_start = 24,
		  .select_end = 24},
		27,
	},
	{
		"bad_command",
		EDITOR_KEY_COMMAND "selt" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 6, .mode = SPECASM_MODE_COMMAND },
		1,
	},
	{
		"delete_select_last",
		test_data EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_UP EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "d" EDITOR_KEY_ENTER,
		formatted_last_deleted,
		"",
		{ .line = 25, .row = 21, .command_col = 3 },
		26,
	},
	{
		"overwrite_end_of_line",
		EDITOR_KEY_INSERT "; Here's an example hello the co",
		"; Here's an example hello the co"
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .row = 1,  .ovr = 1},
		2,
	},
	{
		"bad enter",
		"1" EDITOR_KEY_ENTER,
		"1                                ",
		"",
		{ .line = 0, .row = 0, .col = 1, .editing = 1},
		1,
	},
	{
		"bad enter_ovr",
		"1" EDITOR_KEY_INSERT EDITOR_KEY_ENTER,
		"1                                ",
		"",
		{ .line = 0, .row = 0, .col = 1, .editing = 1, .ovr = 1},
		1,
	},
	{
		"bad down",
		EDITOR_KEY_ENTER EDITOR_KEY_UP "1" EDITOR_KEY_DOWN,
		"1                                "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 1, .editing = 1},
		2,
	},
	{
		"load_save",
		test_data EDITOR_KEY_COMMAND "s file" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "l file" EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .command_col = 8 },
		27,
	},
	{
		"load_save2",
		test_data EDITOR_KEY_COMMAND "s file.x" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "l file.x" EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .command_col = 10 },
		27,
	},
	{
		"up_empty",
		EDITOR_KEY_UP,
		EDITOR_BLANK_LINE,
		"",
		{ 0 },
		1,
	},
	{
		"bad_up",
		EDITOR_KEY_ENTER "1" EDITOR_KEY_UP,
		EDITOR_BLANK_LINE
		"1                               ",
		"",
		{ .col = 1, .line = 1, .row = 1, .editing = 1 },
		2,
	},
	{
		"empty_command",
		EDITOR_KEY_COMMAND EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .command_col = 2 },
		1,
	},
	{
		"select_no_up",
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER EDITOR_KEY_UP,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .command_col = 5,
		  .mode = SPECASM_MODE_SELECT },
		1,
	},
	{
		"delete_edit",
		"; 1245" EDITOR_KEY_ENTER EDITOR_KEY_DELETE,
		"; 1245                          ",
		"",
		{ .line = 0, .row = 0, .col = 6, .editing = 1 },
		1,
	},
	{
		"delete_edit_2",
		EDITOR_KEY_ENTER "; 1245" EDITOR_KEY_ENTER EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE EDITOR_KEY_DELETE EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE EDITOR_KEY_DELETE EDITOR_KEY_DELETE
		EDITOR_KEY_DELETE,
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 0, .editing = 0 },
		1,
	},
	{
		"delete_line_0",
		EDITOR_KEY_ENTER "; 1245" EDITOR_KEY_ENTER EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_DELETE,
		"; 1245                          "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 0, .editing = 0 },
		2,
	},
	{
		"delete_ovr_move_up",
		"; 1245" EDITOR_KEY_ENTER EDITOR_KEY_INSERT
		EDITOR_KEY_DELETE,
		"; 1245                          "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 0, .row = 0, .col = 6, .editing = 1, .ovr = 1 },
		2,
	},
	{
		"delete_ovr_cant_move_up",
		"; 1245" EDITOR_KEY_ENTER "1" EDITOR_KEY_INSERT EDITOR_KEY_LEFT
		EDITOR_KEY_DELETE,
		"; 1245                          "
		"1                               ",
		"",
		{ .line = 1, .row = 1, .col = 0, .editing = 1, .ovr = 1 },
		2,
	},
	{
		"goto_neg",
		test_data EDITOR_KEY_COMMAND "g -1" EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .line = 0, .row = 0, .col = 0, .command_col = 6 },
		27,
	},
	{
		"goto_same_screen",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "g 7"
		EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .line = 7, .row = 7, .col = 0, .command_col = 5 },
		27,
	},
	{
		"goto_almost_end",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "g &18"
		EDITOR_KEY_ENTER,
		&formatted_test_data[4 *  SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 24, .row = 20, .col = 0, .command_col = 7 },
		27,
	},
	{
		"goto_too_big",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "g &fff"
		EDITOR_KEY_ENTER,
		&formatted_test_data[4 *  SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22, .col = 0, .command_col = 8 },
		27,
	},
	{
		"go_nowhere",
		test_data EDITOR_BUF_START EDITOR_KEY_DOWN EDITOR_KEY_COMMAND
		"g 1" EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .line = 1, .row = 1, .col = 0, .command_col = 5 },
		27,
	},
	{
		"empty_count",
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 3 },
		1,
	},
	/*
	 * Unfotunately, there's no way to check the count is correct as this
	 * information is flashed briefly and overwritten.  We'll need to test
	 * the byte count in the linker tests.
	 */
	{
		"byte_count",
		"ld a, 10" EDITOR_KEY_ENTER
		"equb 10" EDITOR_KEY_ENTER
		"bit 3, (ix + 127)" EDITOR_KEY_ENTER
		"@hello" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "b" EDITOR_KEY_ENTER,
		"  ld a, 10                      "
		"equb 10                         "
		"  bit 3, (ix+127)               "
		"@hello                          "
		EDITOR_BLANK_LINE,
		"",
		{ .command_col = 3 },
		5,
	},
	{
		"copy_into_selected_area",
		"ld a, 10" EDITOR_KEY_ENTER
		"equb 10" EDITOR_KEY_ENTER
		"bit 3, (ix + 127)" EDITOR_KEY_ENTER
		"@hello" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_ENTER
		EDITOR_KEY_DOWN
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		"  ld a, 10                      "
		"  ld a, 10                      "
		"equb 10                         "
		"equb 10                         "
		"  bit 3, (ix+127)               "
		"@hello                          "
		EDITOR_BLANK_LINE,
		"",
		{ .row = 1, .line = 1, .command_col = 3 },
		7,
	},
	{
		"copy_above_selected_area",
		"ld a, 10" EDITOR_KEY_ENTER
		"equb 10" EDITOR_KEY_ENTER
		"bit 3, (ix + 127)" EDITOR_KEY_ENTER
		"@hello" EDITOR_KEY_ENTER
		EDITOR_KEY_UP
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		"equb 10                         "
		"  bit 3, (ix+127)               "
		"  ld a, 10                      "
		"equb 10                         "
		"  bit 3, (ix+127)               "
		"@hello                          "
		EDITOR_BLANK_LINE,
		"",
		{ .row = 0, .line = 0, .command_col = 3 },
		7,
	},
	{
		"copy_below_selected_area",
		"ld a, 10" EDITOR_KEY_ENTER
		"equb 10" EDITOR_KEY_ENTER
		"bit 3, (ix + 127)" EDITOR_KEY_ENTER
		"@hello" EDITOR_KEY_ENTER
		EDITOR_BUF_START
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_ENTER
		EDITOR_BUF_END
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		"  ld a, 10                      "
		"equb 10                         "
		"  bit 3, (ix+127)               "
		"@hello                          "
		"  ld a, 10                      "
		"equb 10                         "
		EDITOR_BLANK_LINE,
		"",
		{ .row = 4, .line = 4, .command_col = 3 },
		7,
	},
	{
		"copy_whole_buffer",
		test_data EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		&formatted_test_data2x[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22, .command_col = 3 },
		54,
	},
	{
		"copy_oom",
		test_data EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "a" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		&formatted_test_data2x[4 * SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 26, .row = 22, .command_col = 3, .select_end = 432 },
		432,
	},
	{
		"move_on_top",
		test_data EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_COMMAND"sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_ENTER
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_COMMAND "m" EDITOR_KEY_ENTER,
		&formatted_test_data[4 *  SPECASM_LINE_MAX_LEN],
		"",
		{ .line = 22, .command_col = 3, .row = 18 },
		27,
	},
	{
		"move_below",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		";10" EDITOR_KEY_ENTER
		";11" EDITOR_KEY_ENTER
		";12" EDITOR_KEY_ENTER
		";13" EDITOR_KEY_ENTER
		";14" EDITOR_KEY_ENTER
		EDITOR_BUF_START EDITOR_KEY_DOWN
		EDITOR_KEY_COMMAND"sel" EDITOR_KEY_ENTER
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_ENTER
		EDITOR_BUF_END
		EDITOR_KEY_COMMAND "m" EDITOR_KEY_ENTER,
		";1                              "
		";5                              "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		";10                             "
		";11                             "
		";12                             "
		";13                             "
		";14                             "
		";2                              "
		";3                              "
		";4                              "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 14, .command_col = 3, .row = 14 },
		15,
	},
	{
		"move_above",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		";10" EDITOR_KEY_ENTER
		";11" EDITOR_KEY_ENTER
		";12" EDITOR_KEY_ENTER
		";13" EDITOR_KEY_ENTER
		";14" EDITOR_KEY_ENTER
		EDITOR_BUF_END EDITOR_KEY_UP
		EDITOR_KEY_COMMAND"sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_ENTER
		EDITOR_BUF_START EDITOR_KEY_DOWN
		EDITOR_KEY_COMMAND "m" EDITOR_KEY_ENTER,
		";1                              "
		";11                             "
		";12                             "
		";13                             "
		";2                              "
		";3                              "
		";4                              "
		";5                              "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		";10                             "
		";14                             "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .command_col = 3, .row = 1 },
		15,
	},
	{
		"blank_line_bug",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		EDITOR_KEY_ENTER EDITOR_KEY_ENTER
		";3" EDITOR_KEY_UP EDITOR_KEY_LINE_START
		EDITOR_KEY_DELETE,
		";1                              "
		";2                              "
		EDITOR_BLANK_LINE
		";3                              "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 2, .row = 2 },
		4,
	},
	{
		"copy_overwrite_1",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		";10" EDITOR_KEY_ENTER
		";11" EDITOR_KEY_ENTER
		";12" EDITOR_KEY_ENTER
		";13" EDITOR_KEY_ENTER
		";14" EDITOR_KEY_ENTER
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_UP
		EDITOR_KEY_UP EDITOR_KEY_ENTER EDITOR_KEY_INSERT
		EDITOR_BUF_START EDITOR_KEY_DOWN
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		";1                              "
		";11                             "
		";12                             "
		";13                             "
		";14                             "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		";10                             "
		";11                             "
		";12                             "
		";13                             "
		";14                             "
		EDITOR_BLANK_LINE,
		"",
		{ .line = 1, .command_col = 3, .row = 1, .ovr = 1 },
		15,
	},
	{
		"copy_overwrite_2",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		";10" EDITOR_KEY_ENTER
		";11" EDITOR_KEY_ENTER
		";12" EDITOR_KEY_ENTER
		";13" EDITOR_KEY_ENTER
		";14" EDITOR_KEY_ENTER
		EDITOR_BUF_START
		EDITOR_KEY_COMMAND "sel" EDITOR_KEY_ENTER
		EDITOR_KEY_DOWN EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_DOWN EDITOR_KEY_ENTER EDITOR_KEY_INSERT
		EDITOR_BUF_END EDITOR_KEY_UP
		EDITOR_KEY_COMMAND "c" EDITOR_KEY_ENTER,
		";1                              "
		";2                              "
		";3                              "
		";4                              "
		";5                              "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		";10                             "
		";11                             "
		";12                             "
		";13                             "
		";1                              "
		";2                              "
		";3                              "
		";4                              ",
		"",
		{ .line = 13, .command_col = 3, .row = 13, .ovr = 1 },
		17,
	},
	{
		"find",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "f next"
		EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .line = 7, .row = 7, .col = 0, .command_col = 8 },
		27,
	},
	{
		"no find",
		test_data EDITOR_BUF_START EDITOR_KEY_COMMAND "f nothere"
		EDITOR_KEY_ENTER,
		formatted_test_data,
		"",
		{ .line = 0, .row = 0, .col = 0, .command_col = 11 },
		27,
	},
	{
		"invalidate_select",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		EDITOR_KEY_UP EDITOR_KEY_UP EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_ENTER EDITOR_BUF_START EDITOR_KEY_ENTER,
		"                                "
		";1                              "
		";2                              "
		";3                              "
		";4                              "
		";5                              "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		"                                ",
		"",
		{ .line = 1, .row = 1, .command_col = 5},
		11,
	},
	{
		"dont_invalidate_select",
		";1" EDITOR_KEY_ENTER
		";2" EDITOR_KEY_ENTER
		";3" EDITOR_KEY_ENTER
		";4" EDITOR_KEY_ENTER
		";5" EDITOR_KEY_ENTER
		";6" EDITOR_KEY_ENTER
		";7" EDITOR_KEY_ENTER
		";8" EDITOR_KEY_ENTER
		";9" EDITOR_KEY_ENTER
		EDITOR_BUF_START EDITOR_KEY_COMMAND "sel"
		EDITOR_KEY_ENTER EDITOR_KEY_DOWN EDITOR_KEY_DOWN
		EDITOR_KEY_ENTER EDITOR_KEY_ENTER,
		";1                              "
		";2                              "
		"                                "
		";3                              "
		";4                              "
		";5                              "
		";6                              "
		";7                              "
		";8                              "
		";9                              "
		"                                ",
		"",
		{ .line = 3, .row = 3, .command_col = 5, .select_start = 0,
		  .select_end = 2 },
		11,
	},
};

/* clang-format on */

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
	for (size_t i = 0; i < sizeof(editor_tests) / sizeof(editor_test_t);
	     i++)
		if (prv_run_test(&editor_tests[i]))
			return 1;

	return 0;
}
