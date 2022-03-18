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

#include "peer.h"
#include "peer_file.h"
#include "state.h"

#include <stdio.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

static char file_buf[MAX_BUFFER_SIZE];

static int prv_check_file(const char *fname)
{
	char *period = strrchr(fname, '.');

	if (!period)
		goto on_error;

	if ((period[1] == 'x' || period[1] == 'X') && period[2] == 0)
		return 0;

on_error:
	fprintf(stderr, ".x extension expected got %s\n", fname);

	return 1;
}

static int prv_write_source_file(const char *fname)
{
	specasm_handle_t f;
	uint16_t i;
	size_t ptr;
	char obj_file[SPECASM_PATH_MAX];

	if (strlen(fname) + 1 > SPECASM_PATH_MAX) {
		fprintf(stderr, "Path to long\n");
		return 1;
	}

	strcpy(obj_file, fname);
	obj_file[strlen(fname) - 1] = 's';

	f = specasm_file_wopen_e(obj_file);
	if (err_type != SPECASM_ERROR_OK) {
		fprintf(stderr, "Unable to open source file %s\n", fname);
		return 1;
	}

	ptr = 0;
	for (i = 0; i < state.lines.num_lines; i++) {
		if (ptr + SPECASM_LINE_MAX_LEN + 1 > MAX_BUFFER_SIZE) {
			specasm_file_write_e(f, file_buf, ptr);
			if (err_type != SPECASM_ERROR_OK)
				goto cleanup;
			ptr = 0;
		}
		specasm_format_line_e(&file_buf[ptr], i);
		if (err_type != SPECASM_ERROR_OK)
			goto cleanup;
		ptr += (SPECASM_LINE_MAX_LEN - 1);
		while (file_buf[ptr] == ' ')
			ptr--;
		ptr++;
		file_buf[ptr++] = '\n';
	}

	if (ptr > 0)
		specasm_file_write_e(f, file_buf, ptr);

	specasm_file_close_e(f);
	if (err_type != SPECASM_ERROR_OK)
		return 1;

	return 0;

cleanup:

	(void)specasm_file_close_e(f);

	/*
	 * TODO: unlink the file.
	 */

	return 0;
}

static int prv_load_file(const char *fname)
{
	specasm_load_e(fname);
	if (err_type != SPECASM_ERROR_OK) {
		fprintf(stderr, "Failed to load %s: %s\n", fname,
			specasm_error_msg(err_type));
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: saexport .x\n");
		return 1;
	}

	specasm_init_dump_table();

	for (int i = 1; i < argc; i++) {
		if (prv_check_file(argv[i]))
			return 1;

		if (prv_load_file(argv[i]))
			return 1;

		if (prv_write_source_file(argv[i]))
			return 1;
	}

	return 0;
}
