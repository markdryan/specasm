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

static uint8_t prv_calibration_loop(uint8_t k, uint16_t delay,
				    uint8_t calibration)
{
	uint8_t new_key;
	uint8_t i;

	for (i = 0; i < calibration; i++) {
		specasm_sleep_ms(delay);
		new_key = in_inkey();

		if ((k >= 'A') && (k <= 'Z') && ((k | 32) == new_key))
			continue;

		if (k != new_key)
			return new_key;
	}

	return k;
}

static void prv_main_loop(void)
{
	uint8_t k;
	uint8_t new_key;
	const uint16_t delay = ((200 / 11) * 11) / 10;

	do {
		in_wait_key();
		k = in_inkey();
		if (!k)
			continue;
		do {
			if (k == SPECASM_KEY_COMMAND) {
				in_wait_nokey();
				specasm_handle_key_press(k);
				break;
			}

			new_key = prv_calibration_loop(k, delay,
						       SPECASM_KEY_CALIBRATION);

			/* Check for common key escapes on the toastrack */

			if (((k == '5') && (new_key == SPECASM_KEY_LEFT)) ||
			    ((k == '8') && (new_key == SPECASM_KEY_RIGHT)) ||
			    ((k == '0') && (new_key == SPECASM_KEY_DELETE))) {
				k = new_key;
				new_key = 0;
			}

			specasm_handle_key_press(k);

			if (k == new_key) {
				do {
					new_key = prv_calibration_loop(
					    k, delay / 2,
					    SPECASM_KEY_CALIBRATION / 2);
					if (k != new_key)
						break;
					specasm_handle_key_press(k);
				} while (1);
			}

			k = new_key;
		} while (k);
	} while (!quitting);
}

int main()
{
	uint8_t k;
	uint8_t new_key;
	uint16_t i;
	uint8_t *ptr = (uint8_t *)23328;
	uint16_t delay = ((200 / 11) * *ptr) / 10;

	specasm_init_dump_table();

	zx_border(SPECASM_LABEL_BORDER);
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	err_type = SPECASM_ERROR_OK;
	specasm_editor_reset();

	specasm_draw_status();

	// Make cursor flash
	specasm_text_set_flash(col, line, FLASH);

	prv_main_loop();

	return 0;
}
