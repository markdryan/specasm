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

#include <input.h>
#include <intrinsic.h>
#include <string.h>
#include <z80.h>

#include "editor.h"
#include "line.h"
#include "state.h"

#define SPECASM_KEY_CALIBRATION 13

int main()
{
	uint8_t k;
	uint8_t new_key;
	uint16_t i;

	zx_border(SPECASM_LABEL_BORDER);
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	err_type = SPECASM_ERROR_OK;
	specasm_editor_reset();

	specasm_draw_status();

	// Make cursor flash
	specasm_text_set_flash(col, line, FLASH);
	intrinsic_ei();
	do {
		in_wait_key();
		k = in_inkey();
		if (!k)
			continue;
		do {
			if (k == SPECASM_KEY_COMMAND) {
				in_wait_nokey();
				new_key = in_inkey();
			} else {
				for (i = 0; i < SPECASM_KEY_CALIBRATION; i++) {
					specasm_sleep_ms(20);
					new_key = in_inkey();
					if (k != new_key)
						break;
				}
			}
			specasm_handle_key_press(k);
			k = new_key;
		} while (k);
	} while (!quitting);

	zx_border(INK_WHITE);
	zx_cls(PAPER_WHITE | INK_BLACK);

	return 0;
}
