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
#include <stdlib.h>
#include <string.h>

#include "test_content.h"
#include "test_content_zx.h"

static int prv_make_opcode_file(const char *path)
{
	size_t i;
	char *fname;
	int retval = 1;
	FILE *f = NULL;
	uint16_t count;
	const char *name = "test_op";

	fname = malloc(strlen(path) + strlen(name) + 2);
	if (!fname)
		return 1;

	sprintf(fname, "%s/%s", path, name);

	f = fopen(fname, "w");
	if (!f) {
		fprintf(stderr, "Unable to open %s for writing\n", fname);
		goto cleanup;
	}

	count = (uint16_t)opcode_tests_count;
	if (fwrite(&count, sizeof(count), 1, f) < 1) {
		fprintf(stderr, "Failed to write to %s\n", fname);
		goto cleanup;
	}

	for (i = 0; i < opcode_tests_count; i++) {
		const test_t *t = &opcode_tests[i];
		test_zx_t zx_test;

		if (strlen(t->source) > SPECASM_LINE_MAX_LEN) {
			fprintf(stderr, "%s too long\n", t->source);
			goto cleanup;
		}

		if (strlen(t->str) > SPECASM_LINE_MAX_LEN) {
			fprintf(stderr, "%s too long\n", t->str);
			goto cleanup;
		}

		strcpy(zx_test.source, t->source);
		strcpy(zx_test.str, t->str);
		zx_test.size = (uint8_t)t->size;
		memcpy(zx_test.op_code, t->op_code, 4);

		if (fwrite(&zx_test, sizeof(zx_test), 1, f) < 1) {
			fprintf(stderr, "Failed to write to %s\n", fname);
			goto cleanup;
		}
	}

	retval = fclose(f);
	f = NULL;

cleanup:

	if (f)
		(void)fclose(f);

	free(fname);

	return retval;
}

static int prv_make_bad_test_file(const char *path)
{
	size_t i;
	char *fname;
	int retval = 1;
	FILE *f = NULL;
	uint16_t count;
	const char *name = "test_bad";

	fname = malloc(strlen(path) + strlen(name) + 2);
	if (!fname)
		return 1;

	sprintf(fname, "%s/%s", path, name);

	f = fopen(fname, "w");
	if (!f) {
		fprintf(stderr, "Unable to open %s for writing\n", fname);
		goto cleanup;
	}

	count = (uint16_t)bad_tests_count;
	if (fwrite(&count, sizeof(count), 1, f) < 1) {
		fprintf(stderr, "Failed to write to %s\n", fname);
		goto cleanup;
	}

	for (i = 0; i < bad_tests_count; i++) {
		const bad_test_t *t = &bad_tests[i];
		bad_test_zx_t zx_test;

		if (strlen(t->source) > SPECASM_LINE_MAX_LEN) {
			fprintf(stderr, "%s too long\n", t->source);
			goto cleanup;
		}

		strcpy(zx_test.source, t->source);
		zx_test.error = t->error;

		if (fwrite(&zx_test, sizeof(zx_test), 1, f) < 1) {
			fprintf(stderr, "Failed to write to %s\n", fname);
			goto cleanup;
		}
	}

	retval = fclose(f);
	f = NULL;

cleanup:

	if (f)
		(void)fclose(f);

	free(fname);

	return retval;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: test_content_zx path\n");
		return 1;
	}

	int retval =
	    prv_make_opcode_file(argv[1]) | prv_make_bad_test_file(argv[1]);

	return retval;
}
