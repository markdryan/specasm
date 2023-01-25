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

#ifndef SPECASM_ERROR_H
#define SPECASM_ERROR_H

#include <stdint.h>

#define SPECASM_ERROR_OK 0
#define SPECASM_ERROR_BAD_MNENOMIC 1
#define SPECASM_ERROR_TOO_MANY_LINES 2
#define SPECASM_ERROR_TOO_MANY_SHORT_STRINGS 3
#define SPECASM_ERROR_TOO_MANY_LONG_STRINGS 4
#define SPECASM_ERROR_STRING_TOO_LONG 5
#define SPECASM_ERROR_ASSERT_BAD_STRING_ID 6
#define SPECASM_ERROR_LONG_LABEL_EX 7
#define SPECASM_ERROR_BAD_COMMENT 8
#define SPECASM_ERROR_BAD_LINES 9
#define SPECASM_ERROR_NO_ROOM_IN_LINE 10
#define SPECASM_ERROR_BAD_COMMAND 11
#define SPECASM_ERROR_OPEN 12
#define SPECASM_ERROR_WRITE 13
#define SPECASM_ERROR_READ 14
#define SPECASM_ERROR_CORRUPT 15
#define SPECASM_ERROR_BAD_REG 16
#define SPECASM_ERROR_BAD_NUM 17
#define SPECASM_ERROR_COMMA_EXPECTED 18
#define SPECASM_ERROR_CONDITION_CODE 19
#define SPECASM_ERROR_BAD_LABEL 20
#define SPECASM_ERROR_NUM_TOO_BIG 21
#define SPECASM_ERROR_NUM_NEG 22
#define SPECASM_ERROR_BAD_FNAME 23
#define SPECASM_ERROR_SPECASM_TOO_OLD 24
#define SPECASM_ERROR_BAD_EXPRESSION 25
#define SPECASM_MAX_ERRORS 26

typedef uint8_t specasm_error_t;

extern specasm_error_t err_type;

extern const char *error_msgs[SPECASM_MAX_ERRORS];

#define specasm_error_init(err) err = SPECASM_ERROR_OK
#define specasm_error_msg(err) (error_msgs[err])

#endif
