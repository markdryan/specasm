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

#ifndef SPECASM_EDITOR_H
#define SPECASM_EDITOR_H

#include <stdint.h>

#include "peer.h"

#define SPECASM_KEY_LEFT 8
#define SPECASM_KEY_RIGHT 9
#define SPECASM_KEY_DOWN 10
#define SPECASM_KEY_UP 11
#define SPECASM_KEY_DELETE 12
#define SPECASM_KEY_ENTER 13
#define SPECASM_KEY_INSERT 130     /* SYMSHIFT + i */
#define SPECASM_KEY_LINE_START 131 /* SYMSHIFT + q */
#define SPECASM_KEY_COMMAND 132    /* SYMSHIFT + w */
#define SPECASM_KEY_LINE_END 133   /* SYMSHIFT + e */
#define SPECASM_KEY_BUF_START 7    /* CAPSHIFT + 1 */
#define SPECASM_KEY_BUF_END 6      /* CAPSHIFT + 2 */
#define SPECASM_KEY_PAGE_UP 128    /* CAPSHIFT + 3 */
#define SPECASM_KEY_PAGE_DOWN 129  /* CAPSHIFT + 4 */

#define SPECASM_MODE_EDITOR 0
#define SPECASM_MODE_SELECT 1
#define SPECASM_MODE_COMMAND 2

extern unsigned int line;
extern uint8_t col;
extern uint8_t quitting;

#ifdef SPECASM_TARGET_NEXT
/*
 * Used to preload a .x file to edit on startup.  Assumes that the screen has
 * already been cleared.
 */
void specasm_editor_preload(const char *fname);
#endif
void specasm_draw_status(void);
void specasm_handle_key_press(uint8_t k);
void specasm_editor_reset(void);

#endif
