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

#ifndef SPECASM_ANALYSIS_H
#define SPECASM_ANALYSIS_H

#include <stdint.h>
#include "line.h"

struct specasm_cycles_t_ {
	uint8_t m[2];
	uint8_t t[2];
};

typedef struct specasm_cycles_t_ specasm_cycles_t;

void specasm_get_cycles(const specasm_line_t *l, specasm_cycles_t* cycles);
uint8_t specasm_get_flags(const specasm_line_t *l);

#endif
