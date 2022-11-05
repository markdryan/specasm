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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "peer.h"

char peer_unit_screen[SPECASM_UNIT_BUF_SZ];
uint8_t peer_unit_atts[SPECASM_UNIT_BUF_SZ];

void specasm_text_set_flash(uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *aptr = &peer_unit_atts[(y * SPECASM_LINE_MAX_LEN) + x];

	*aptr &= ~((uint8_t)SPECASM_FLASH);
	*aptr |= attr;
}

void specasm_text_printch(char ch, uint8_t x, uint8_t y, uint8_t attr)
{
	size_t offset = (y * SPECASM_LINE_MAX_LEN) + x;
	char *cptr = &peer_unit_screen[offset];
	uint8_t *aptr = &peer_unit_atts[offset];

	*cptr = ch;
	*aptr = attr;
}

uint8_t specasm_text_print(const char *str, uint8_t x, uint8_t y, uint8_t attr)
{
	size_t offset = (y * SPECASM_LINE_MAX_LEN) + x;
	size_t len = strlen(str);
	char *cptr = &peer_unit_screen[offset];
	uint8_t *aptr = &peer_unit_atts[offset];

	memcpy(cptr, str, len);
	memset(aptr, attr, len);

	return (uint8_t)len;
}

void specasm_text_clear(uint8_t x, uint8_t y, uint8_t l, uint8_t attr)
{
	size_t offset = (y * SPECASM_LINE_MAX_LEN) + x;
	char *cptr = &peer_unit_screen[offset];
	uint8_t *aptr = &peer_unit_atts[offset];

	memset(cptr, ' ', l);
	memset(aptr, attr, l);
}

void specasm_cls(uint8_t a)
{
	memset(peer_unit_screen, ' ', sizeof(peer_unit_screen) - 1);
	memset(peer_unit_atts, a, sizeof(peer_unit_atts) - 1);
}

void specasm_border(uint8_t a) {}

void specasm_screen_flush(uint16_t peer_last_row)
{
	char line[SPECASM_LINE_MAX_LEN + 1];

	line[SPECASM_LINE_MAX_LEN] = 0;
	for (size_t i = 0; i <= peer_last_row; i++) {
		size_t offset = (i * SPECASM_LINE_MAX_LEN);
		memcpy(line, &peer_unit_screen[offset], SPECASM_LINE_MAX_LEN);
		printf("%s\n", line);
	}
}

int itoa(int n, char *s, unsigned char radix)
{
	return sprintf(s, radix == 16 ? "%" PRIX16 : "%" PRId16, (int16_t)n);
}
int utoa(int n, char *s, unsigned char radix)
{
	return sprintf(s, radix == 16 ? "%" PRIX16 : "%" PRIu16, (uint16_t)n);
}
