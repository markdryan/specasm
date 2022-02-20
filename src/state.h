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

#ifndef SPECASM_STATE_H
#define SPECASM_STATE_H

#include "state_base.h"

extern char scratch[SPECASM_MAX_SCRATCH];

uint8_t specasm_state_add_short_e(const char *str);
uint8_t specasm_state_add_long_e(const char *str);
void specasm_state_check_label_e(const char *str);
void specasm_set_comment(unsigned int l, const char *str);
/*
 * str is expected to contain exactly SPECASM_LINE_MAX_LEN
 * characters.  It does not need to be NULL terminated, but
 * characters not used must be filled with space characters.
 */

void specasm_parse_line_e(unsigned int line, const char *str);
void specasm_append_empty_line_e(void);
void specasm_delete_lines(unsigned int start, unsigned int end);
void specasm_insert_lines_e(unsigned int l, unsigned int count);
void specasm_format_line_e(char *buf, unsigned int l);

#endif
