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

#ifndef SPECASM_EDITOR_EXTRA_H
#define SPECASM_EDITOR_EXTRA_H

void specasm_selecting_clip_copy_e(void);
uint8_t specasm_selecting_clip_cut_e(void);
void specasm_selecting_clip_paste_e(void);
void specasm_selecting_cycles(void);
void specasm_selecting_flags(void);
void specasm_garbage_collect_e(void);

#endif
