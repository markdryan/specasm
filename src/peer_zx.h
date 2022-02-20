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

#ifndef SPECASM_ZX_PEER_H
#define SPECASM_ZX_PEER_H

#include <arch/zx.h>
#include <z80.h>

#define SPECASM_SALINK_BACKGROUND PAPER_WHITE
#define SPECASM_SALINK_BORDER INK_WHITE
#define SPECASM_LABEL_BORDER INK_BLACK
#define SPECASM_LABEL_BACKGROUND PAPER_BLACK
#define SPECASM_LABEL_COLOUR (BRIGHT | INK_CYAN | SPECASM_LABEL_BACKGROUND)
#define SPECASM_CODE_COLOUR (BRIGHT | INK_WHITE | SPECASM_LABEL_BACKGROUND)
#define SPECASM_COMMENT_COLOUR (BRIGHT | INK_GREEN | SPECASM_LABEL_BACKGROUND)
#define SPECASM_DATA_COLOUR (BRIGHT | INK_MAGENTA | SPECASM_LABEL_BACKGROUND)
#define SPECASM_ERROR_COLOUR (PAPER_RED | INK_BLACK)
#define SPECASM_SUCCESS_COLOUR (PAPER_GREEN | INK_BLACK)
#define SPECASM_SELECT_COLOUR (PAPER_WHITE | INK_BLACK)
#define SPECASM_STATUS_COLOUR (PAPER_WHITE | INK_BLACK)
#define SPECASM_HEADER_COLOUR (PAPER_BLACK | INK_WHITE)

#define SPECASM_FLASH FLASH
#define specasm_cls(a) zx_cls(a)
#define specasm_border(a) zx_border(a)
#define specasm_sleep_ms(a) specasm_halt_n(a / 20)

void specasm_halt_n(unsigned int a);

#endif
