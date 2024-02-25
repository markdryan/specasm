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

#ifdef SPECTRUM
#include <arch/zx.h>
#else
#include <arch/zxn.h>
#endif
#include <input.h>
#include <z80.h>

#include "editor.h"
#include "specasm_mainloop.h"

static uint8_t prv_calibration_loop(uint8_t k, uint16_t delay,
				    uint8_t calibration)
{
	uint8_t new_key;
	uint8_t i;
	uint16_t old_scan_code = in_key_scancode(k) & 0x3ff;

	for (i = 0; i < calibration; i++) {
		specasm_sleep_ms(delay);
		new_key = in_inkey();
		if (old_scan_code != (in_key_scancode(new_key) & 0x3ff))
			return new_key;
	}

	return k;
}

void specasm_main_loop(uint16_t delay, uint8_t calibration)
{
	uint8_t k;
	uint8_t new_key;

	do {
		in_wait_key();

		/*
		 * Delay here is needed on the Toastrack to avoid
		 * phantom keys when pressing things like Delete or the
		 * arrow keys.
		 */

		specasm_sleep_ms(delay / 2);
		k = in_inkey();
		if (!k)
			continue;
		do {
			new_key = prv_calibration_loop(k, delay, calibration);
			specasm_handle_key_press(k);

			while (k == new_key) {
				new_key =
					prv_calibration_loop(k,
						delay / 2,
						calibration / 2);
				if (k == new_key)
					specasm_handle_key_press(k);
			}

			k = new_key;
		} while (k);
	} while (!quitting);
}
