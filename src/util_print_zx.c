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

#include <arch/zx.h>

#include "util_print_zx.h"

uint8_t specasm_util_print(const char *str, uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *cptr;
	uint8_t *aptr = zx_cxy2aaddr(x, y);
	uint8_t *bsptr;
	uint8_t *sptr;
	uint8_t i;
	uint8_t old_x = x;

	bsptr = zx_cxy2saddr(x, y);
	while (*str) {
		*aptr++ = attr;
		cptr = (uint8_t *)15360 + (*((uint8_t *)str) * 8);
		sptr = bsptr;
		i = 8;
		do {
			*sptr = *cptr++;
			sptr += 256;
			i--;
		} while (i != 0);
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
