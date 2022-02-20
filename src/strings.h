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

#ifndef SPECASM_STRINGS_H
#define SPECASM_STRINGS_H

#include <stdint.h>

// String length includes the \0

#define SPECASM_MAX_SHORT_STRINGS 128
#define SPECASM_MAX_SHORT_LEN 12

#define SPECASM_MAX_LONG_STRINGS 32
#define SPECASM_MAX_LONG_LEN 32

/*
 * Strings are aligned by their size and are NULL terminated.
 */
struct specasm_short_strings_t_ {
	char strs[SPECASM_MAX_SHORT_STRINGS * SPECASM_MAX_SHORT_LEN];
	uint8_t num_strings;
};
typedef struct specasm_short_strings_t_ specasm_short_strings_t;

struct specasm_long_strings_t_ {
	char strs[SPECASM_MAX_LONG_STRINGS * SPECASM_MAX_LONG_LEN];
	uint8_t num_strings;
};
typedef struct specasm_long_strings_t_ specasm_long_strings_t;

#endif
