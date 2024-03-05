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
#include "state.h"

#include <arch/zxn/esxdos.h>
#include <errno.h>
#include <intrinsic.h>

void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	unsigned char f;

	errno = 0;
	f = esx_f_open(fname, ESX_MODE_W | ESX_MODE_OPEN_CREAT);
	if (errno) {
		err_type = SPECASM_ERROR_OPEN;
		return;
	}

	(void)esx_f_write(f, &state, sizeof(state));
	if (errno) {
		err_type = SPECASM_ERROR_WRITE;
		goto cleanup;
	}

	(void)esx_f_write(f, &checksum, sizeof(checksum));
	if (errno)
		err_type = SPECASM_ERROR_WRITE;

cleanup:
	esx_f_close(f);
}

uint16_t specasm_peer_read_state_e(const char *fname)
{
	unsigned char f;
	uint16_t checksum = 0;

	errno = 0;
	f = esx_f_open(fname, ESX_MODE_R);
	if (errno) {
		err_type = SPECASM_ERROR_OPEN;
		return 0;
	}

	if (esx_f_read(f, &state, sizeof(state)) < sizeof(state)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

	if (esx_f_read(f, &checksum, sizeof(checksum)) < sizeof(checksum)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

on_error:
	esx_f_close(f);

	return checksum;
}
