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

#include <arch/zxn.h>

#include "util_print_zx.h"

#define SPECASM_NEXT_NUM_CHARS 112

/*
 * Reserve some space for the spectrum font copied from the
 * character set rendered to the screen during the dotn startup.
 * We can't acccess the character set directly as it's in the ROM
 * which is mapped out.
 */

static uint8_t specasm_font[SPECASM_NEXT_NUM_CHARS * 8];

static void prv_write_chars(void) __naked
{
__asm
	push af
	push bc
	push hl

	ld hl, 0x5800

	ld a, 22
	rst 16
	ld a, 0
	rst 16
	ld a, 0
	rst 16
	ld a, 32
	ld b, SPECASM_NEXT_NUM_CHARS
CHAR_LOOP:
	push af
	push hl
	push bc
	rst 16
	pop bc
	pop hl
	xor a
	ld (hl), a
	inc l
	pop af
	inc a
	djnz CHAR_LOOP

	pop hl
	pop bc
	pop af
	ret
__endasm;
}

void specasm_peer_next_copy_chars(void)
{
	uint8_t i;
	uint8_t j;
	const uint8_t *new_sptr;
	const uint8_t *sptr = zx_cxy2saddr(0, 0);
	uint8_t *byte = &specasm_font[0];

	prv_write_chars();

	for (i = 0; i < SPECASM_NEXT_NUM_CHARS; i++) {
		new_sptr = sptr;
		for (j = 0; j < 8; j++) {
			*byte = *new_sptr;
			byte++;
			new_sptr += 256;
		}
		sptr++;
	}
}

uint8_t specasm_util_print(const char *str, uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *cptr;
	uint8_t *aptr = zx_cxy2aaddr(x, y);
	uint8_t *bsptr;
	uint8_t *sptr;
	uint8_t i;
	uint8_t ch;
	uint8_t old_x = x;

	bsptr = zx_cxy2saddr(x, y);
	while (*str) {
		*aptr++ = attr;
		ch = *((uint8_t*) str);
		if (ch >= 32 && ch <= 143) {
			cptr = &specasm_font[(ch - 32) * 8];
			sptr = bsptr;
			i = 8;
			do {
				*sptr = *cptr++;
				sptr += 256;
				i--;
			} while (i != 0);
		}
		str++;
		x++;
		bsptr++;
	}

	return x - old_x;
}

void specasm_util_clear(uint8_t x, uint8_t y, uint8_t l, uint8_t attr)
{
	uint8_t *aptr = zx_cxy2aaddr(x, y);
	uint8_t *sptr;
	uint8_t *bsptr;
	uint8_t i;

	bsptr = zx_cxy2saddr(x, y);
	while (l > 0) {
		*aptr++ = attr;
		sptr = bsptr;
		i = 8;
		do {
			*sptr = 0;
			sptr += 256;
			i--;
		} while (i != 0);
		x++;
		l--;
		bsptr++;
	}
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

	if (ch < 32 || ch > 143)
		return;

	*aptr = attr;
	cptr = &specasm_font[(((uint8_t)ch) - 32) * 8];
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
