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

#include <input.h>
#include <intrinsic.h>
#include <string.h>
#include <z80.h>

#include "editor.h"
#include "line.h"
#include "state.h"

extern unsigned char _SYSVAR_LASTK;

int main()
{
	uint8_t k;

	specasm_init_dump_table();

	zx_border(SPECASM_LABEL_BORDER);
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	err_type = SPECASM_ERROR_OK;
	specasm_editor_reset();

	specasm_draw_status();

	// Make cursor flash
	specasm_text_set_flash(col, line, FLASH);

	intrinsic_ei();
	in_wait_nokey();
	do {
		_SYSVAR_LASTK = 0;
		for (; k = _SYSVAR_LASTK; k == 0);
		specasm_handle_key_press(k);
	}
	while (!quitting);
	in_wait_nokey();

	return 0;
}
