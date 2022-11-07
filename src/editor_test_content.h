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

#ifndef EDITOR_TEST_CONTENT_H
#define EDITOR_TEST_CONTENT_H

#include <stddef.h>
#include <stdint.h>

struct editor_test_state_t_ {
	unsigned int line;
	uint8_t col;
	uint8_t row;
	uint8_t command_col;
	uint8_t ovr;
	uint8_t editing;
	uint8_t quitting;
	uint8_t mode;
	unsigned int select_start;
	unsigned int select_end;
};
typedef struct editor_test_state_t_ editor_test_state_t;

struct editor_test_t_ {
	const char *name;
	const char *input;
	const char *screen;
	const char *atts;
	editor_test_state_t state;
	unsigned int num_lines;
	unsigned int command_test;
};
typedef struct editor_test_t_ editor_test_t;

extern const editor_test_t editor_tests[];
extern const size_t editor_tests_count;

#endif
