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

#include "editor_tests.h"
#include "error.h"
#include "line.h"
#include "state.h"
#include "test_content.h"

static int prv_test_opcodes()
{
	size_t i;
	uint8_t parsed;

	err_type = SPECASM_ERROR_OK;

	for (i = 0; i < opcode_tests_count; i++) {
		char buf[33] = {0};
		specasm_line_t line = {0};
		const test_t *t = &opcode_tests[i];

		printf("opcode: %s : ", t->source);
		parsed = specasm_parse_mnemomic_e(t->source, 0, &line);
		if (err_type != SPECASM_ERROR_OK) {
			printf("[FAIL]\n\t>%s\n", error_msgs[err_type]);
			return 1;
		}
		if (parsed != strlen(t->source)) {
			printf(
			    "[FAIL]\n\t> mismatched length %d expected %ld\n",
			    parsed, strlen(t->source));
			return 1;
		}

		if (t->size != specasm_line_get_size(&line) + 1) {
			printf(
			    "[FAIL]\n\t> mismatched opcode length %d expected"
			    " got %d\n",
			    t->size, specasm_line_get_size(&line) + 1);
			return 1;
		}

		if (memcmp(line.data.op_code, t->op_code, 4)) {
			printf("[FAIL]\n\t> mismatched opcode %2x%2x%2x%2x "
			       "expected"
			       " got %2x%2x%2x%2x\n",
			       t->op_code[0], t->op_code[1], t->op_code[2],
			       t->op_code[3], line.data.op_code[0],
			       line.data.op_code[1], line.data.op_code[2],
			       line.data.op_code[3]);
			return 1;
		}

		parsed = specasm_dump_opcode_e(&line, buf);
		if (err_type != SPECASM_ERROR_OK) {
			printf("[FAIL]\n\t>dump: %s\n", error_msgs[err_type]);
			return 1;
		}
		if (strcmp(t->str, buf)) {
			printf(
			    "[FAIL] bad format.  Expected \"%s\" got \"%s\"\n",
			    t->str, buf);
			return 1;
		}

		printf("[OK]\n");
	}

	return 0;
}

static int prv_test_parse_format_bare()
{
	size_t i;

	err_type = SPECASM_ERROR_OK;

	for (i = 0; i < opcode_tests_count; i++) {
		char buf[SPECASM_LINE_MAX_LEN + 1];
		char buf2[SPECASM_LINE_MAX_LEN + 1];
		const test_t *t = &opcode_tests[i];
		const specasm_line_t *line = &state.lines.lines[0];
		uint8_t type;

		memset(buf, ' ', sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = 0;
		memcpy(buf, t->source, strlen(t->source));

		printf("opcode: %s : ", t->source);
		specasm_parse_line_e(0, buf);
		if (err_type != SPECASM_ERROR_OK) {
			printf("[FAIL]\n\t>%s\n", error_msgs[err_type]);
			return 1;
		}

		if (t->size != specasm_line_get_size(line) + 1) {
			printf(
			    "[FAIL]\n\t> mismatched opcode length %d expected"
			    " got %d\n",
			    t->size, specasm_line_get_size(line) + 1);
			return 1;
		}

		if (memcmp(line->data.op_code, t->op_code, 4)) {
			printf("[FAIL]\n\t> mismatched opcode %2x%2x%2x%2x "
			       "expected"
			       " got %2x%2x%2x%2x\n",
			       t->op_code[0], t->op_code[1], t->op_code[2],
			       t->op_code[3], line->data.op_code[0],
			       line->data.op_code[1], line->data.op_code[2],
			       line->data.op_code[3]);
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
			printf("[FAIL]\n\t>dump: %s\n", error_msgs[err_type]);
			return 1;
		}
		if (strcmp(buf, buf2)) {
			printf("[FAIL] bad format.  Expected \n\"%s\" got "
			       "\n\"%s\"\n",
			       buf2, buf);
			return 1;
		}

		printf("[OK]\n");
	}

	return 0;
}

static int prv_test_format()
{
	size_t i;

	err_type = SPECASM_ERROR_OK;

	for (i = 0; i < format_tests_count; i++) {
		char buf[SPECASM_LINE_MAX_LEN + 1];
		char buf2[SPECASM_LINE_MAX_LEN + 1];
		const format_test_t *t = &format_tests[i];
		const specasm_line_t *line = &state.lines.lines[0];

		printf("test: %s : ", t->source);
		memset(buf, ' ', sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = 0;
		if (strlen(t->source) > SPECASM_LINE_MAX_LEN) {
			printf("[ASSERT] test string too long\n");
			return 1;
		}
		memcpy(buf, t->source, strlen(t->source));

		specasm_parse_line_e(0, buf);
		if (err_type != SPECASM_ERROR_OK) {
			printf("[FAIL]\n\t>%s\n", error_msgs[err_type]);
			return 1;
		}

		if (line->type != t->type) {
			printf("[FAIL]\n\t>Line type mismatch %d expected got "
			       "%d\n",
			       t->type, line->type);
			return 1;
		}

		memset(buf, 0, sizeof(buf));
		memset(buf2, ' ', sizeof(buf2) - 1);
		buf2[sizeof(buf2) - 1] = 0;
		memcpy(buf2, t->str, strlen(t->str));
		specasm_format_line_e(buf, 0);
		if (err_type != SPECASM_ERROR_OK) {
			printf("[FAIL]\n\t>dump: %s\n", error_msgs[err_type]);
			return 1;
		}
		if (strcmp(buf, buf2)) {
			printf("[FAIL] bad format.  Expected \n\"%s\" got "
			       "\n\"%s\"\n",
			       buf2, buf);
			return 1;
		}

		printf("[OK]\n");
	}

	return 0;
}

static int prv_test_bad_opcodes()
{
	size_t i;

	for (i = 0; i < bad_tests_count; i++) {
		char buf[SPECASM_LINE_MAX_LEN + 1];
		const bad_test_t *t = &bad_tests[i];
		err_type = SPECASM_ERROR_OK;

		printf("bad opcode: %s : ", t->source);

		memset(buf, ' ', sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = 0;
		if (strlen(t->source) > SPECASM_LINE_MAX_LEN) {
			printf("[ASSERT] test string too long\n");
			return 1;
		}
		memcpy(buf, t->source, strlen(t->source));

		(void)specasm_parse_line_e(0, buf);
		if (err_type != t->error) {
			printf("[FAIL]\n\t>expected %s got %s\n",
			       error_msgs[t->error], error_msgs[err_type]);
			return 1;
		}

		printf("[OK]\n");
	}

	return 0;
}

static int prv_test_old_version()
{
	err_type = SPECASM_ERROR_OK;
	printf("old_version_check: ");
	specasm_state_reset();
	state.version = SPECASM_VERSION + 1;
	specasm_save_e("hello");
	specasm_load_e("hello");
	if (err_type != SPECASM_ERROR_SPECASM_TOO_OLD) {
		printf("[FAIL]\n\t>expected %s got %s\n",
		       error_msgs[SPECASM_ERROR_SPECASM_TOO_OLD],
		       error_msgs[err_type]);
		return 1;
	}

	printf("[OK]\n");
	return 0;
}

int main(int argc, char *argv[])
{
	specasm_init_dump_table();

	if (prv_test_opcodes())
		return 1;

	printf("\n");
	if (prv_test_parse_format_bare())
		return 1;

	printf("\n");
	if (prv_test_format())
		return 1;

	printf("\n");
	if (prv_test_bad_opcodes())
		return 1;

	printf("\n");
	if (prv_test_old_version())
		return 1;

	printf("\n");
	if (run_editor_tests())
		return 1;

	return 0;
}
