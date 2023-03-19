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

#ifndef SPECASM_STATE_READ_H
#define SPECASM_STATE_READ_H

#define SPECASM_VERSION 6
#define SPECASM_VERSION_STR "v6"

#include <stdint.h>

#include "line.h"
#include "strings.h"

struct specasm_state_t_ {
	specasm_lines_t lines;
	specasm_short_strings_t short_strs;
	specasm_long_strings_t long_strs;
	uint16_t version;
};
typedef struct specasm_state_t_ specasm_state_t;

extern specasm_state_t state;

void specasm_state_reset(void);

const char *specasm_state_get_short_e(uint8_t i);
const char *specasm_state_get_long_e(uint8_t i);

void specasm_load_e(const char *fname);
void specasm_save_e(const char *fname);

uint16_t specasm_compute_line_size(specasm_line_t *line);

#endif
