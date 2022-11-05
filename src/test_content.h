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

#ifndef TEST_CONTENT_H
#define TEST_CONTENT_H

#include <stddef.h>
#include <stdint.h>

#include "error.h"

struct test_t_ {
	const char *source;
	const char *str;
	unsigned int size;
	uint8_t op_code[4];
};
typedef struct test_t_ test_t;

struct format_test_t_ {
	const char *source;
	const char *str;
	uint8_t type;
};
typedef struct format_test_t_ format_test_t;

struct bad_test_t_ {
	const char *source;
	specasm_error_t error;
};
typedef struct bad_test_t_ bad_test_t;

extern const test_t opcode_tests[];
extern const size_t opcode_tests_count;

extern const format_test_t format_tests[];
extern const size_t format_tests_count;

extern const bad_test_t bad_tests[];
extern const size_t bad_tests_count;

#endif
