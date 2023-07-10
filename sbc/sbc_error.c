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

#include "sbc_error.h"

specasm_error_t err_type;

static const char* const sbc_error_msgs[] = {
	"Bad program", // SBC_ERROR_BAD_PROGRAM
	"Bad number",  // SBC_ERROR_BAD_NUM
	"Missing \"", // SBC_ERROR_MISSING_QUOTE
	"Failed to open file", // SBC_ERROR_OPEN
	"Failed to write to file", // SBC_ERROR_WRITE
	"Failed to read from file", // SBC_ERROR_READ
	"Too many constant integers", // SBC_ERROR_TOO_MANY_INTS
	"Too many constant reals", // SBC_ERROR_TOO_MANY_REALS
	"Too many constant strings", // SBC_ERROR_TOO_MANY_STRINGS
	"Too many expressions", // SBC_ERROR_TOO_MANY_EXPRESSIONS
	"Bad line number", // SBC_ERROR_BAD_LABEL
	"Program too big", // SBC_ERROR_PROG_TOO_BIG
	"Keyword expected", // SBC_ERROR_KEYWORD_EXPECTED
	"Identifier expected", // SBC_ERROR_ID_EXPECTED
	"= expected", // SBC_ERROR_EQ_EXPECTED
	"Expression expected", // SBC_ERROR_EXP_EXPECTED
	"Too many nested levels", // SBC_ERROR_TOO_MUCH_NESTING
	"ENDWHILE unexpected", // SBC_ERROR_ENDWHILE_UNEXPECTED
	"Line number expected", // SBC_ERROR_LINENO_EXPECTED
	", expected", // SBC_ERROR_COMMA_EXPECTED
	"TO expected", // SBC_ERROR_TO_EXPECTED
	"NEXT unexpected", // SBC_ERROR_NEXT_UNEXPECTED
	") expected", // SBC_ERROR_CLOSEB_UNEXPECTED
	"ENDCASE expected", // SBC_ERROR_ENDCASE_UNEXPECTED
	"OF unexpected", // SBC_ERROR_OF_UNEXPECTED
};

const char *sbc_error_msg(void)
{
	if ((err_type >= SBC_ERROR_BAD_PROGRAM) &&
	    (err_type < SBC_ERROR_MAX)) {
		return sbc_error_msgs[err_type - SBC_ERROR_BAD_PROGRAM];
	}

	return "Unknown error";
}
