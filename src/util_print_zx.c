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

void specasm_text_set_flash(uint8_t x, uint8_t y, uint8_t attr)
{
	uint8_t *aptr = zx_cxy2aaddr(x, y);

	*aptr &= ~((uint8_t)FLASH);
	*aptr |= attr;
}

void specasm_screen_flush(uint16_t peer_last_row)
{
	uint8_t *posn_x = (uint8_t *)23688;
	uint8_t *posn_y = (uint8_t *)23689;

	*posn_x = 33;
	*posn_y = 24 - peer_last_row;
}
