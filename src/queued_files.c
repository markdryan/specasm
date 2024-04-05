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
#include "queued_files.h"

static void prv_add_queued_filename_e(const char *base, const char *prefix,
				      const char *str);

static uint8_t prv_add_queued_dir_e(const char *fname)
{
	specasm_dir_t dir;
	specasm_dirent_t dirent;
	size_t fname_len;
	char *period;
	char *ptr;
	char dir_name[MAX_FNAME + 1];

	/*
	 * If we can't stat the filename we'll assume that it's just a .x file
	 * without the extension.  If it isn't will catch the error later.
	 */

	if (!specasm_file_isdir(fname))
		return 0;

	fname_len = strlen(fname);
	if (fname_len >= MAX_FNAME) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return 0;
	}
	dir = specasm_opendir_e(fname);
	if (err_type != SPECASM_ERROR_OK) {
		strcpy(error_buf, "Failed to read directory");
		err_type = SALINK_ERROR_READDIR;
		return 0;
	}

	strcpy(dir_name, fname);
	ptr = &dir_name[fname_len];
	ptr[0] = '/';
	ptr[1] = 0;

	while (specasm_readdir(dir, &dirent)) {

		/*
		 * Make sure we only add .x files and not .x directories
		 * or any directories for that matter.  Subdirectories need
		 * to be explicitly added with a '-' or a '+' directive.
		 */

		if (specasm_isdirent_dir(dirent))
			continue;

		/*
		 * We don't use salink_check_file here as .t files must be
		 * explictly included.  They are not included when including
		 * a directory.
		 */

		period = strchr(specasm_getdirname(dirent), '.');
		if (!period || !period[1] || period[2])
			continue;

		if (!((period[1] == 'x') || (period[1] == 'X')))
			continue;

		prv_add_queued_filename_e(dir_name, "",
					  specasm_getdirname(dirent));
		if (err_type != SPECASM_ERROR_OK) {
			specasm_closedir(dir);
			return 0;
		}
	}
	specasm_closedir(dir);

	err_type = SPECASM_ERROR_OK;

	return 1;
}

static void prv_add_queued_filename_e(const char *base, const char *prefix,
				      const char *str)
{
	int space_needed;
	int prefix_len;
	int base_len;
	char *ptr;
	char *start;
	char *slash;

	if (queued_files == MAX_PENDING_X_FILES) {
		snprintf(error_buf, sizeof(error_buf),
			 "Pending file limit %d reached", MAX_PENDING_X_FILES);
		err_type = SALINK_ERROR_TOO_MANY_FILES;
		return;
	}

	/*
	 * Build up a path relative to the including path, providing
	 * it's not a complete path.  So if the including file is
	 * one/two.x and it includes does - three.x, then we get
	 *
	 * one/three.x
	 *
	 * If the including file is simple two.x then we get
	 *
	 * three.x
	 *
	 * If the including file is one/two/ we'd get
	 *
	 * one/two/three.x
	 *
	 * TODO: Need to update this to cope with drive letters.
	 */

	base_len = 0;
	if (str[0] != '/') {
		slash = strrchr(base, '/');
		if (slash)
			base_len = (slash - base) + 1;
	}

	prefix_len = strlen(prefix);
	space_needed = strlen(str) + prefix_len + base_len;
	if (space_needed > MAX_FNAME) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return;
	}

	ptr = &buf.fname[queued_files][0];
	start = ptr;
	if (base_len) {
		strncpy(ptr, base, base_len);
		ptr += base_len;
	}
	if (prefix_len) {
		strcpy(ptr, prefix);
		ptr += prefix_len;
	}
	strcpy(ptr, str);

	/*
	 * Check to see whether this is a directory or not.
	 * If so we add the files in this directory and return.
	 */

	if (prv_add_queued_dir_e(start))
		return;
	if (err_type != SPECASM_ERROR_OK)
		return;

	ptr = &start[space_needed - 2];

	/*
	 * .t files can be explicitly included but shouldn't be added to the
	 * main binary.
	 */

	if (ptr[0] == '.' && (ptr[1] | 32) == 't') {
		if (link_mode == SALINK_MODE_LINK) {
			got_test = 1;
			return;
		}
	}
	queued_files++;
	ptr = &start[space_needed - 2];
	if (ptr[0] != '.' && (ptr[1] | 32) != 'x') {
		if (space_needed + 2 > MAX_FNAME) {
			err_type = SPECASM_ERROR_BAD_FNAME;
			return;
		}
		ptr[2] = '.';
		ptr[3] = 'x';
		ptr[4] = 0;
	}
}

#ifdef SPECASM_NEXT_BANKED
void salink_add_queued_file_banked_e(const char *base, const char *prefix,
				     specasm_line_t *line)
#else
void salink_add_queued_file_e(const char *base, const char *prefix,
			      specasm_line_t *line)
#endif
{
	uint8_t id;
	const char *str;

	id = line->data.label;
	str = salink_get_label_str_e(id, line->type);
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_add_queued_filename_e(base, prefix, str);
}
