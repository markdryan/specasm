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

#include "peer.h"
#include "state.h"

void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	FILE *f;

	f = fopen(fname, "w");
	if (!f) {
		err_type = SPECASM_ERROR_OPEN;
		return;
	}

	if (fwrite(&state, 1, sizeof(state), f) < sizeof(state)) {
		err_type = SPECASM_ERROR_WRITE;
		goto cleanup;
	}

	if (fwrite(&checksum, 1, sizeof(checksum), f) < sizeof(checksum))
		err_type = SPECASM_ERROR_WRITE;

cleanup:

	if (fclose(f))
		err_type = SPECASM_ERROR_WRITE;
}

uint16_t specasm_peer_read_state_e(const char *fname)
{
	FILE *f;
	uint16_t checksum = 0;

	f = fopen(fname, "r");
	if (!f) {
		err_type = SPECASM_ERROR_OPEN;
		return 0;
	}

	if (fread(&state, 1, sizeof(state), f) < sizeof(state)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

	if (fread(&checksum, 1, sizeof(checksum), f) < sizeof(checksum)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

on_error:
	(void)fclose(f);

	return checksum;
}
