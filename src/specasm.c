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
#include "editor_buffers.h"
#include "line.h"
#include "specasm_mainloop.h"
#include "state.h"

#define SPECASM_KEY_CALIBRATION 13

#ifdef SPECASM_TARGET_128
void specasm_trampolines_init(void);
#endif

int main()
{
	uint8_t k;
	uint8_t new_key;
	uint16_t i;
#ifdef SPECASM_TARGET_128
	uint8_t *ptr = (uint8_t *)23328;
#else
	uint8_t *ptr = (uint8_t *)0xbffe;
#endif
	const uint16_t delay = ((200 / 11) * 11) / 10;

#ifdef SPECASM_TARGET_128
	specasm_trampolines_init();
#endif

	specasm_init_dump_table();
	zx_border(SPECASM_LABEL_BORDER);
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	err_type = SPECASM_ERROR_OK;
	specasm_editor_reset();

	specasm_draw_status();

	// Make cursor flash
	specasm_text_set_flash(col, line, FLASH);

	specasm_main_loop(delay, SPECASM_KEY_CALIBRATION);

	return 0;
}
