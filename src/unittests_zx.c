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

#include <arch/zx.h>

#include "error.h"
#include "peer.h"
#include "peer_file.h"
#include "state.h"
#include "test_content.h"
#include "test_content_zx.h"
#include "util_print_zx.h"

#define UNITTEST_ZX_TEST_LINE 0
#define UNITTEST_ZX_FORMAT_LINE 1
#define UNITTEST_ZX_FORMAT_LINE2 2
#define UNITTEST_ZX_ERROR_LINE 3
#define UNITTEST_ZX_OPCODE_OK_LINE 6
#define UNITTEST_ZX_FORMAT_OK_LINE 7
#define UNITTEST_ZX_BAD_OK_LINE 8
#define UNITTEST_ZX_VERSION_OK_LINE 9

static int prv_test_opcode(const test_zx_t *t)
{
	char buf[33] = {0};
	specasm_line_t line = {0};
	uint8_t parsed;

	specasm_util_clear(0, UNITTEST_ZX_TEST_LINE,
			   SPECASM_LINE_MAX_LEN, PAPER_WHITE|INK_BLACK);
	(void) specasm_util_print(t->source, 0,
				  UNITTEST_ZX_TEST_LINE,
				  PAPER_WHITE|INK_BLACK);

	parsed = specasm_parse_mnemomic_e(t->source, 0, &line);
	if (err_type != SPECASM_ERROR_OK) {
		(void) specasm_util_print(error_msgs[err_type], 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (parsed != strlen(t->source)) {
		(void) specasm_util_print("mismatched length", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (t->size != specasm_line_get_size(&line) + 1) {
		(void) specasm_util_print("mismatched opcode length", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (memcmp(line.data.op_code, t->op_code, 4)) {
		(void) specasm_util_print("mismatched opcode", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
			return 1;
	}

	parsed = specasm_dump_opcode_e(&line, buf);
	if (err_type != SPECASM_ERROR_OK) {
		(void) specasm_util_print(error_msgs[err_type], 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}
	buf[parsed] = 0;
	if (strcmp(t->str, buf)) {
		(void) specasm_util_print(buf, 0, UNITTEST_ZX_FORMAT_LINE,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print("bad format", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	return 0;
}


static int prv_test_format(const test_zx_t *t)
{
	size_t i;

	char buf[SPECASM_LINE_MAX_LEN + 1];
	char buf2[SPECASM_LINE_MAX_LEN + 1];
	const specasm_line_t *line = &state.lines.lines[0];
	uint8_t type;

	memset(buf, ' ', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	memcpy(buf, t->source, strlen(t->source));

	specasm_util_clear(0, UNITTEST_ZX_TEST_LINE,
			   SPECASM_LINE_MAX_LEN, PAPER_WHITE|INK_BLACK);
	(void) specasm_util_print(t->source, 0,
				  UNITTEST_ZX_TEST_LINE,
				  PAPER_WHITE|INK_BLACK);

	specasm_parse_line_e(0, buf);
	if (err_type != SPECASM_ERROR_OK) {
		(void) specasm_util_print(error_msgs[err_type], 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (t->size != specasm_line_get_size(line) + 1) {
		(void) specasm_util_print("mismatched opcode length", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (memcmp(line->data.op_code, t->op_code, 4)) {
		(void) specasm_util_print("mismatched opcode", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
			return 1;
	}

	memset(buf, 0, sizeof(buf));
	memset(buf2, ' ', sizeof(buf2) - 1);
	buf2[sizeof(buf2) - 1] = 0;
	type = specasm_line_get_adj_type(line);
	if (type != SPECASM_LINE_TYPE_DB &&
	    type != SPECASM_LINE_TYPE_DW &&
	    type != SPECASM_LINE_TYPE_DB_SUB &&
	    type != SPECASM_LINE_TYPE_DW_SUB &&
	    type != SPECASM_LINE_TYPE_DS)
		memcpy(buf2 + SPECASM_MAX_INDENT, t->str,
		       strlen(t->str));
	else
		memcpy(buf2, t->str, strlen(t->str));
	specasm_format_line_e(buf, 0);
	if (err_type != SPECASM_ERROR_OK) {
		(void) specasm_util_print("dump: ", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print(error_msgs[err_type], 6,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	if (strcmp(buf, buf2)) {
		(void) specasm_util_print(buf, 0, UNITTEST_ZX_FORMAT_LINE,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print(buf2, 0, UNITTEST_ZX_FORMAT_LINE2,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print("bad format", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	return 0;
}

static int prv_test_bad(const bad_test_zx_t *t)
{
	size_t i;
	char buf[SPECASM_LINE_MAX_LEN + 1];

	err_type = SPECASM_ERROR_OK;
	specasm_util_clear(0, UNITTEST_ZX_TEST_LINE,
			   SPECASM_LINE_MAX_LEN, PAPER_WHITE|INK_BLACK);
	(void) specasm_util_print(t->source, 0,
				  UNITTEST_ZX_TEST_LINE,
				  PAPER_WHITE|INK_BLACK);
	memset(buf, ' ', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = 0;
	memcpy(buf, t->source, strlen(t->source));

	(void)specasm_parse_line_e(0, buf);
	if (err_type != t->error) {
		(void) specasm_util_print(error_msgs[err_type], 0,
					  UNITTEST_ZX_FORMAT_LINE,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print(error_msgs[t->error], 0,
					  UNITTEST_ZX_FORMAT_LINE2,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print("bad error", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}
	err_type = SPECASM_ERROR_OK;

	return 0;
}

static int prv_opcode_tests(void)
{
	specasm_handle_t h;
	uint16_t count;
	test_zx_t test;

	h = specasm_file_ropen_e("test_op");
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	(void) specasm_file_read_e(h, &count, 2);
	if (err_type != SPECASM_ERROR_OK)
		goto close_file;

	for (uint16_t i = 0; i < count; i++) {
		(void) specasm_file_read_e(h, &test, sizeof(test));
		if (err_type != SPECASM_ERROR_OK)
			goto close_file;

		if (prv_test_opcode(&test)) {
			specasm_file_close_e(h);
			return 1;
		}
	}

	(void) specasm_util_print("opcode tests [ok]", 0,
				  UNITTEST_ZX_OPCODE_OK_LINE,
				  PAPER_WHITE|INK_BLACK);

	specasm_file_close_e(h);

	return 0;

close_file:
	specasm_file_close_e(h);

on_error:
	(void) specasm_util_print(error_msgs[err_type], 0,
				  UNITTEST_ZX_ERROR_LINE,
				  PAPER_RED|INK_BLACK);
	return 1;
}

static int prv_format_tests(void)
{
	specasm_handle_t h;
	uint16_t count;
	test_zx_t test;

	h = specasm_file_ropen_e("test_op");
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	(void) specasm_file_read_e(h, &count, 2);
	if (err_type != SPECASM_ERROR_OK)
		goto close_file;

	for (uint16_t i = 0; i < count; i++) {
		(void) specasm_file_read_e(h, &test, sizeof(test));
		if (err_type != SPECASM_ERROR_OK)
			goto close_file;

		if (prv_test_format(&test)) {
			specasm_file_close_e(h);
			return 1;
		}
	}

	(void) specasm_util_print("format tests [ok]", 0,
				  UNITTEST_ZX_FORMAT_OK_LINE,
				  PAPER_WHITE|INK_BLACK);

	specasm_file_close_e(h);

	return 0;

close_file:
	specasm_file_close_e(h);

on_error:
	(void) specasm_util_print(error_msgs[err_type], 0,
				  UNITTEST_ZX_ERROR_LINE,
				  PAPER_RED|INK_BLACK);
	return 1;
}

static int prv_bad_tests(void)
{
	specasm_handle_t h;
	uint16_t count;
	bad_test_zx_t test;

	h = specasm_file_ropen_e("test_bad");
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	(void) specasm_file_read_e(h, &count, 2);
	if (err_type != SPECASM_ERROR_OK)
		goto close_file;

	for (uint16_t i = 0; i < count; i++) {
		(void) specasm_file_read_e(h, &test, sizeof(test));
		if (err_type != SPECASM_ERROR_OK)
			goto close_file;

		if (prv_test_bad(&test)) {
			specasm_file_close_e(h);
			return 1;
		}
	}

	(void) specasm_util_print("bad tests [ok]", 0,
				  UNITTEST_ZX_BAD_OK_LINE,
				  PAPER_WHITE|INK_BLACK);

	specasm_file_close_e(h);

	return 0;

close_file:
	specasm_file_close_e(h);

on_error:
	(void) specasm_util_print(error_msgs[err_type], 0,
				  UNITTEST_ZX_ERROR_LINE,
				  PAPER_RED|INK_BLACK);
	return 1;
}

static int prv_test_old_version(void)
{
	err_type = SPECASM_ERROR_OK;

	specasm_util_clear(0, UNITTEST_ZX_TEST_LINE,
			   SPECASM_LINE_MAX_LEN, PAPER_WHITE|INK_BLACK);
	(void) specasm_util_print("version check", 0,
				  UNITTEST_ZX_TEST_LINE,
				  PAPER_WHITE|INK_BLACK);

	specasm_state_reset();
	state.version = SPECASM_VERSION + 1;
	specasm_save_e("hello");
	specasm_load_e("hello");
	if (err_type != SPECASM_ERROR_SPECASM_TOO_OLD) {
		(void) specasm_util_print(
			error_msgs[SPECASM_ERROR_SPECASM_TOO_OLD], 0,
			UNITTEST_ZX_FORMAT_LINE, PAPER_RED|INK_BLACK);
		(void) specasm_util_print(error_msgs[err_type], 0,
					  UNITTEST_ZX_FORMAT_LINE2,
					  PAPER_RED|INK_BLACK);
		(void) specasm_util_print("bad error", 0,
					  UNITTEST_ZX_ERROR_LINE,
					  PAPER_RED|INK_BLACK);
		return 1;
	}

	(void) specasm_util_print("version test [ok]", 0,
				  UNITTEST_ZX_VERSION_OK_LINE,
				  PAPER_WHITE|INK_BLACK);

	return 0;
}

int main(int argc, char *argv[])
{
	specasm_init_dump_table();

	specasm_error_init(err_type);
	zx_border(INK_WHITE);
	zx_cls(PAPER_WHITE | INK_BLACK);

	if (prv_opcode_tests() || prv_format_tests() || prv_bad_tests() ||
	    prv_test_old_version())
		return 1;

	return 0;
}

