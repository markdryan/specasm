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

#include "error.h"

specasm_error_t err_type;

/* clang-format off */

const char* error_msgs[SPECASM_MAX_ERRORS] = {
	"OK",                     // SPECASM_ERROR_OK
	"Bad mnenomic",           // SPECASM_ERROR_BAD_MNENOMIC
	"Too many lines",         // SPECASM_ERROR_TOO_MANY_LINES
	"Too many short strings", // SPECASM_ERROR_TOO_MANY_SHORT_STRINGS
	"Too many long strings",  // SPECASM_ERROR_TOO_MANY_LONG_STRINGS
	"String too long",        // SPECASM_ERROR_STRING_TOO_LONG
	"ASSERT: bad string id",  // SPECASM_ERROR_ASSERT_BAD_STRING_ID
	"Long labels cannot share a line",  // SPECASM_ERROR_LONG_LABEL_EX
	"Bad comment",            // SPECASM_ERROR_BAD_COMMENT
	"One or more lines are invalid", // SPECASM_ERROR_BAD_LINES
	"No room to insert characters", // SPECASM_ERROR_NO_ROOM_IN_LINE
	"Bad command", // SPECASM_ERROR_BAD_COMMAND
	"Failed to open file", // SPECASM_ERROR_OPEN
	"Failed to write to file", // SPECASM_ERROR_WRITE
	"Failed to read from file", // SPECASM_ERROR_READ
	"Not a valid specasm file", // SPECASM_ERROR_CORRUPT
	"Bad register", // SPECASM_BAD_REG
	"Bad number", // SPECASM_BAD_NUM
	"Comma expected", // SPECASM_ERROR_COMMA_EXPECTED
	"Bad condition code", // SPECASM_ERROR_CONDITION_CODE
	"Bad label", // SPECASM_ERROR_BAD_LABEL
	"Number too big", // SPECASM_ERROR_NUM_TOO_BIG
	"Negative number", // SPECASM_ERROR_NUM_NEG
	"Bad filename", // SPECASM_ERROR_BAD_FNAME
	"SPECASM too old", // SPECASM_ERROR_SPECASM_TOO_OLD
};
