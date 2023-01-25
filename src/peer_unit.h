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

#ifndef SPECASM_UNIT_PEER_H
#define SPECASM_UNIT_PEER_H

#include "line.h"

#define SPECASM_SALINK_BACKGROUND 1
#define SPECASM_SALINK_BORDER 1
#define SPECASM_LABEL_BORDER 1
#define SPECASM_LABEL_BACKGROUND 1
#define SPECASM_LABEL_COLOUR (0x10 | SPECASM_LABEL_BACKGROUND)
#define SPECASM_CODE_COLOUR (0x20 | SPECASM_LABEL_BACKGROUND)
#define SPECASM_COMMENT_COLOUR (0x40 | SPECASM_LABEL_BACKGROUND)
#define SPECASM_DATA_COLOUR (0x80 | SPECASM_LABEL_BACKGROUND)
#define SPECASM_EQU_COLOUR (0x82 | SPECASM_LABEL_BACKGROUND)
#define SPECASM_ERROR_COLOUR (0x22)
#define SPECASM_SUCCESS_COLOUR (0x25)
#define SPECASM_SELECT_COLOUR (0x24)
#define SPECASM_STATUS_COLOUR (0x24)
#define SPECASM_HEADER_COLOUR (0x25)

#define SPECASM_FLASH 0x80
#define specasm_sleep_ms(a)

#define SPECASM_UNIT_BUF_SZ ((SPECASM_MAX_ROWS + 1) * SPECASM_LINE_MAX_LEN + 1)

extern char peer_unit_screen[SPECASM_UNIT_BUF_SZ];
extern uint8_t peer_unit_atts[SPECASM_UNIT_BUF_SZ];

void specasm_cls(uint8_t a);
void specasm_border(uint8_t a);

#endif
