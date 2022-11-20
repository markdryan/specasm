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

#include <arch/zx/esxdos.h>
#include <intrinsic.h>

void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	unsigned char f;

	f = esxdos_f_open(fname, ESXDOS_MODE_W | ESXDOS_MODE_OC);
	if (!f) {
		err_type = SPECASM_ERROR_OPEN;
		return;
	}

	if (esxdos_f_write(f, &state, sizeof(state)) == 0) {
		err_type = SPECASM_ERROR_WRITE;
		goto cleanup;
	}

	if (esxdos_f_write(f, &checksum, sizeof(checksum)) == 0)
		err_type = SPECASM_ERROR_WRITE;

cleanup:
	esxdos_f_close(f);
}

uint16_t specasm_peer_read_state_e(const char *fname)
{
	unsigned char f;
	uint16_t checksum = 0;

	f = esxdos_f_open(fname, ESXDOS_MODE_R);
	if (!f) {
		err_type = SPECASM_ERROR_OPEN;
		return 0;
	}

	if (esxdos_f_read(f, &state, sizeof(state)) < sizeof(state)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

	if (esxdos_f_read(f, &checksum, sizeof(checksum)) < sizeof(checksum)) {
		err_type = SPECASM_ERROR_READ;
		goto on_error;
	}

on_error:
	esxdos_f_close(f);

	return checksum;
}

void specasm_text_set_flash(uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *aptr = zx_cxy2aaddr(x, y);

	*aptr &= ~((uint8_t)FLASH);
	*aptr |= attr;
}

void specasm_text_printch(char ch, uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *cptr;
	uint8_t *aptr = zx_cxy2aaddr(x, y);
	uint8_t *sptr = zx_cxy2saddr(x, y);
	uint8_t i;

	*aptr = attr;
	cptr = (uint8_t *)15360 + (((uint8_t)ch) * 8);
	i = 8;
	do {
		*sptr = *cptr++;
		sptr += 256;
		i--;
	} while (i != 0);
}

void specasm_screen_flush(uint16_t peer_last_row)
{
	uint8_t *posn_x = (uint8_t *)23688;
	uint8_t *posn_y = (uint8_t *)23689;

	*posn_x = 33;
	*posn_y = 24 - peer_last_row;
}
