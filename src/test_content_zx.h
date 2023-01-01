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

#ifndef TEST_CONTENT_ZX_H
#define TEST_CONTENT_ZX_H

#include <stdint.h>

#include "line.h"

struct test_zx_t_ {
	char source[SPECASM_LINE_MAX_LEN + 1];
	char str[SPECASM_LINE_MAX_LEN + 1];
	uint8_t size;
	uint8_t op_code[4];
};
typedef struct test_zx_t_ test_zx_t;

struct bad_test_zx_t_ {
	char source[SPECASM_LINE_MAX_LEN + 1];
	specasm_error_t error;
};
typedef struct bad_test_zx_t_ bad_test_zx_t;

#endif
