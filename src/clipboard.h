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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdint.h>

void specasm_clip_reset(void);
void specasm_clip_add_line_e(const char *line);
uint16_t specasm_clip_get_line(uint16_t ptr, char *buffer);
uint16_t specasm_clip_get_line_count(void);

#endif
