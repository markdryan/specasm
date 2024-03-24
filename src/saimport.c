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

#include "peer.h"
#include "peer_file.h"
#include "state.h"

#define MAX_BUFFER_SIZE 1024

static char file_buf[MAX_BUFFER_SIZE];
uint16_t bytes_in_buf;
uint16_t ptr;

static int prv_check_file(const char *fname, char *ext)
{
	char *period = strrchr(fname, '.');

	if (!period)
		goto on_error;

	if ((period[1] == 's' || period[1] == 'S') && period[2] == 0) {
	        *ext = 'x';
		return 0;
	}

	if ((period[1] == 't' || period[1] == 'T') &&
	    (period[2] == 's' || period[2] == 'S') &&
	    (period[3] == 0)) {
		*ext = 't';
		return 0;
	}

on_error:
	fprintf(stderr, ".s or .ts extension expected got %s\n", fname);

	return 1;
}

static uint8_t prv_get_line_e(specasm_handle_t f, char *buf, uint8_t *eof)
{
	uint16_t read;
	char ch;
	uint8_t line_len = 0;

	*eof = 0;

	do {
		while (ptr < bytes_in_buf) {
			ch = file_buf[ptr++];
			if (ch == '\n')
				return line_len;
			if (ch < ' ')
				continue;
			if (line_len == SPECASM_LINE_MAX_LEN) {
				err_type = SPECASM_ERROR_NO_ROOM_IN_LINE;
				return 0;
			}
			buf[line_len++] = ch;
		}

		read = specasm_file_read_e(f, file_buf, MAX_BUFFER_SIZE);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		if (read == 0) {
			*eof = 1;
			/*
			 * If we reach EOF and there's some data in our buffer
			 * force a newline.
			 */

			if (line_len == 0)
				return 0;
			file_buf[0] = '\n';
			bytes_in_buf = 1;
		} else {
			bytes_in_buf = read;
		}
		ptr = 0;
	} while (1);
}

static int prv_parse_file(const char *fname)
{
	uint8_t eof;
	uint16_t linelen;
	specasm_handle_t f;
	char buf[SPECASM_LINE_MAX_LEN + 1];
	unsigned int cur_line = 0;
	int retval = 1;

	buf[SPECASM_LINE_MAX_LEN] = 0;
	f = specasm_file_ropen_e(fname);
	if (err_type != SPECASM_ERROR_OK) {
		fprintf(stderr, "Unable to open source file %s\n", fname);
		return 1;
	}

	specasm_state_reset();

	do {
		memset(buf, ' ', SPECASM_LINE_MAX_LEN);
		linelen = prv_get_line_e(f, buf, &eof);
		if (err_type != SPECASM_ERROR_OK) {
			fprintf(stderr, "Failed to read line: %s\n",
				specasm_error_msg(err_type));
			goto cleanup;
		}
		if ((linelen == 0) && eof)
			break;
		specasm_append_empty_line_e();
		if (err_type != SPECASM_ERROR_OK) {
			fprintf(stderr, "%s\n", specasm_error_msg(err_type));
			goto cleanup;
		}
		if (linelen > 0) {
			specasm_parse_line_e(cur_line, buf);
			if (err_type != SPECASM_ERROR_OK) {
				fprintf(stderr, "Syntax error at line %u: %s\n",
					cur_line, specasm_error_msg(err_type));
				goto cleanup;
			}
		}
		cur_line = state.lines.num_lines;
	} while (!eof);

	retval = 0;
cleanup:

	specasm_file_close_e(f);

	return retval;
}

static int prv_write_object_file(const char *fname, char ext)
{
	char obj_file[SPECASM_PATH_MAX];
	char *period;

	if (strlen(fname) + 1 > SPECASM_PATH_MAX) {
		fprintf(stderr, "Path to long\n");
		return 1;
	}

	strcpy(obj_file, fname);

	/*
	 * If we get here we know obj_file has at least 1 full stop.
	 */

	period = strrchr(obj_file, '.');
	period[1] = ext;
	period[2] = 0;

	specasm_save_e(obj_file);
	if (err_type != SPECASM_ERROR_OK) {
		fprintf(stderr, "Failed to write %s: %s\n", obj_file,
			specasm_error_msg(err_type));
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char ext;

	if (argc < 2) {
		fprintf(stderr, "Usage: saimport .s\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (prv_check_file(argv[i], &ext))
			return 1;

		if (prv_parse_file(argv[i]))
			return 1;

		if (prv_write_object_file(argv[i], ext))
			return 1;
	}

	return 0;
}
