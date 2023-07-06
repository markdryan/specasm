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

#ifndef SBC_ERROR_H
#define SBC_ERROR_H

#include "error.h"

#define SBC_ERROR_BAD_PROGRAM SPECASM_MAX_ERRORS
#define SBC_ERROR_BAD_NUM (SPECASM_MAX_ERRORS + 1)
#define SBC_ERROR_MISSING_QUOTE (SPECASM_MAX_ERRORS + 2)
#define SBC_ERROR_OPEN (SPECASM_MAX_ERRORS + 3)
#define SBC_ERROR_WRITE (SPECASM_MAX_ERRORS + 4)
#define SBC_ERROR_READ (SPECASM_MAX_ERRORS + 5)
#define SBC_ERROR_TOO_MANY_INTS (SPECASM_MAX_ERRORS + 6)
#define SBC_ERROR_TOO_MANY_REALS (SPECASM_MAX_ERRORS + 7)
#define SBC_ERROR_TOO_MANY_STRINGS (SPECASM_MAX_ERRORS + 8)
#define SBC_ERROR_TOO_MANY_EXPRESSIONS (SPECASM_MAX_ERRORS + 9)
#define SBC_ERROR_BAD_LABEL (SPECASM_MAX_ERRORS + 10)
#define SBC_ERROR_PROG_TOO_BIG (SPECASM_MAX_ERRORS + 11)
#define SBC_ERROR_KEYWORD_EXPECTED (SPECASM_MAX_ERRORS + 12)
#define SBC_ERROR_ID_EXPECTED (SPECASM_MAX_ERRORS + 13)
#define SBC_ERROR_EQ_EXPECTED (SPECASM_MAX_ERRORS + 14)
#define SBC_ERROR_EXP_EXPECTED (SPECASM_MAX_ERRORS + 15)
#define SBC_ERROR_TOO_MUCH_NESTING (SPECASM_MAX_ERRORS + 16)
#define SBC_ERROR_ENDWHILE_EXPECTED (SPECASM_MAX_ERRORS + 17)
#define SBC_ERROR_MAX (SPECASM_MAX_ERRORS + 18)

const char *sbc_error_msg(void);

#endif
