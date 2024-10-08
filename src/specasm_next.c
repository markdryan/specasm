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
#include <arch/zxn/esxdos.h>

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

void specasm_peer_next_copy_chars(void);

int main(int argc, char *argv[])
{
	uint8_t turbo;
	struct esx_mode mode;
	const uint16_t delay = ((200 / 11) * 11) / 10;

	/*
	 * The editor is nicer to use at 28Mhz.  We're running on
	 * a Next so we might as well use all that power!
	 */

	turbo = ZXN_READ_REG(REG_TURBO_MODE);
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo | 3);

	specasm_init_dump_table();

	memset(&mode, 0, sizeof(mode));
	(void)esx_ide_mode_set(&mode);

	zx_border(SPECASM_LABEL_BORDER);
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);
	specasm_peer_next_copy_chars();
	zx_cls(SPECASM_CODE_COLOUR | SPECASM_LABEL_BACKGROUND);

	err_type = SPECASM_ERROR_OK;

	if (argc > 1)
		specasm_editor_preload(argv[1]);
	else
		specasm_editor_reset();

	specasm_draw_status();

	// Make cursor flash
	specasm_text_set_flash(col, line, FLASH);

	specasm_main_loop(delay, SPECASM_KEY_CALIBRATION);

	zx_cls(PAPER_WHITE | INK_WHITE);
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo);

	return 0;
}
