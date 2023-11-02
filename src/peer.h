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

#ifndef SPECASM_PEER_H
#define SPECASM_PEER_H

#include <stdint.h>

#include "error.h"

#ifdef UNITTESTS
#include "peer_unit.h"
#elif defined(SPECTRUM)
#include "peer_zx.h"
#elif defined(__ZXNEXT)
#include "peer_zx.h"
#endif

void specasm_peer_write_state_e(const char *fname, uint16_t checksum);

uint16_t specasm_peer_read_state_e(const char *fname);

void specasm_text_set_flash(uint8_t x, uint8_t y, uint8_t attr);

void specasm_text_printch(char ch, uint8_t x, uint8_t y, uint8_t attr);

#if !defined(SPECTRUM) && !defined(__ZXNEXT)
int itoa(int n, char *s, unsigned char radix);
int utoa(int n, char *s, unsigned char radix);
#endif

uint8_t specasm_text_print(const char *str, uint8_t x, uint8_t y, uint8_t attr);
void specasm_text_clear(uint8_t x, uint8_t y, uint8_t l, uint8_t attr);

/*
 * Flushes any buffered output and sets the text cursor to
 * peer_last_row.
 */

void specasm_screen_flush(uint16_t peer_last_row);

#endif
