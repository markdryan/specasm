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

	f = esxdos_f_open(fname, ESXDOS_MODE_W | ESXDOS_MODE_CT);
	if (!f)
		err_type = SPECASM_ERROR_OPEN;

	return f;
}

specasm_handle_t specasm_file_ropen_e(const char *fname)
{
	specasm_handle_t f;

	f = esxdos_f_open(fname, ESXDOS_MODE_R);
	if (!f)
		err_type = SPECASM_ERROR_OPEN;

	return f;
}

void specasm_file_write_e(specasm_handle_t f, const void *data, size_t size)
{
	if (esxdos_f_write(f, (void *)data, size) == 0)
		err_type = SPECASM_ERROR_WRITE;
}

size_t specasm_file_read_e(specasm_handle_t f, void *data, size_t size)
{
	/*
	 * I don't see any way to distinguish between an error and an
	 * EOF, so for now this function just returns the bytes read.
	 */

	return esxdos_f_read(f, data, size);
}

void specasm_file_close_e(specasm_handle_t f) { (void)esxdos_f_close(f); }

specasm_dir_t specasm_opendir_e(const char *fname)
{
	specasm_dir_t d = esxdos_f_opendir(fname);
	if (!d)
		err_type = SPECASM_ERROR_OPEN;

	return d;
}

void specasm_file_stat_e(specasm_handle_t f, specasm_stat_t *buf)
{
	if (esxdos_f_fstat(f, buf) < 0)
		err_type = SPECASM_ERROR_READ;
}
