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

#include "peer_file.h"

#include "error.h"

specasm_handle_t specasm_file_wopen_e(const char *fname)
{
	specasm_handle_t f;

	f = fopen(fname, "w");
	if (!f)
		err_type = SPECASM_ERROR_OPEN;

	return f;
}
specasm_handle_t specasm_file_ropen_e(const char *fname)
{
	specasm_handle_t f;

	f = fopen(fname, "r");
	if (!f)
		err_type = SPECASM_ERROR_OPEN;

	return f;
}

void specasm_file_write_e(specasm_handle_t f, const void *data, size_t size)
{
	if (fwrite(data, 1, size, f) < size)
		err_type = SPECASM_ERROR_WRITE;
}

size_t specasm_file_read_e(specasm_handle_t f, void *data, size_t size)
{
	size_t read = fread(data, 1, size, f);

	if (read < size && !feof(f))
		err_type = SPECASM_ERROR_WRITE;

	return read;
}

void specasm_file_close_e(specasm_handle_t f)
{
	if (fclose(f))
		err_type = SPECASM_ERROR_WRITE;
}

specasm_dir_t specasm_opendir_e(const char *fname)
{
	specasm_dir_t d = opendir(fname);
	if (!d)
		err_type = SPECASM_ERROR_OPEN;

	return d;
}

uint8_t specasm_readdir(specasm_dir_t dir, specasm_dirent_t *dirent)
{
	specasm_dirent_t *result;

	result = readdir(dir);
	if (!result)
		return 0;
	*dirent = *result;

	return 1;
}

void specasm_file_stat_e(specasm_handle_t f, specasm_stat_t *buf)
{
	if (fstat(fileno(f), buf))
		err_type = SPECASM_ERROR_READ;
}
