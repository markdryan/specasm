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

#ifndef SALINK_H
#define SALINK_H

#include <stdint.h>

#include "line.h"

#define MAX_FILES 64
#define MAX_GLOBALS 128
#define MAX_LABELS 1280
#define MAX_BUFFER_SIZE 1024
#define MAX_FNAME 28

/*
 * All long labels must be odd.
 */

#define SALINK_LABEL_TYPE_SHORT 0
#define SALINK_LABEL_TYPE_LNG 1
#define SALINK_LABEL_TYPE_ALIGN 2
#define SALINK_LABEL_TYPE_EQU_SHORT 4
#define SALINK_LABEL_TYPE_EQU_LONG 5
#define SALINK_LABEL_TYPE_EQU_GLOBAL 6
#define SALINK_LABEL_TYPE_EQU_EVAL_SHORT 8
#define SALINK_LABEL_TYPE_EQU_EVAL_LONG 9
#define SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL 10

#define SALINK_ERROR_TOO_MANY_LABELS SPECASM_MAX_ERRORS
#define SALINK_ERROR_TOO_MANY_GLOBALS (SPECASM_MAX_ERRORS + 1)
#define SALINK_ERROR_MULTIPLE_DEFS (SPECASM_MAX_ERRORS + 2)
#define SALINK_ERROR_TOO_MANY_FILES (SPECASM_MAX_ERRORS + 3)
#define SALINK_ERROR_NO_MAIN (SPECASM_MAX_ERRORS + 4)
#define SALINK_ERROR_PROGRAM_TOO_BIG (SPECASM_MAX_ERRORS + 5)
#define SALINK_ERROR_UNRESOLVED_LABEL (SPECASM_MAX_ERRORS + 6)
#define SALINK_ERROR_JUMP_TOO_FAR (SPECASM_MAX_ERRORS + 7)
#define SALINK_ERROR_READDIR (SPECASM_MAX_ERRORS + 8)
#define SALINK_ERROR_TOO_MANY_ORGS (SPECASM_MAX_ERRORS + 9)
#define SALINK_ERROR_NEGATIVE_SIZE (SPECASM_MAX_ERRORS + 10)
#define SALINK_ERROR_SIZE_TOO_BIG (SPECASM_MAX_ERRORS + 11)
#define SALINK_ERROR_CANT_OPEN (SPECASM_MAX_ERRORS + 12)
#define SALINK_ERROR_RECURISVE_EQU (SPECASM_MAX_ERRORS + 13)
#define SALINK_ERROR_LOCAL_IN_GLOBAL_EQU (SPECASM_MAX_ERRORS + 14)
#define SALINK_ERROR_BAD_EXP (SPECASM_MAX_ERRORS + 15)
#define SALINK_ERROR_UNEXPECTED_EXP (SPECASM_MAX_ERRORS + 16)
#define SALINK_ERROR_DIV_ZERO (SPECASM_MAX_ERRORS + 17)
#define SALINK_ERROR_BAD_EXPRESSION (SPECASM_MAX_ERRORS + 18)

/*
 * Label usage for EQU statements
 *
 * id holds the id of the label
 *
 * type == SALINK_LABEL_TYPE_EQU_SHORT or SALINK_LABEL_TYPE_EQU_LONG
 *   data holds the type and id of the expression string.
 * type == SALINK_LABEL_TYPE_EQU_EVALUATED
 *   data holds the value of the evaluated expression for local labels
 *
 * Once the value of an EQU statement has been computed its type is changed
 * to SALINK_LABEL_TYPE_EQU_EVALUATED and its value is stored in data.
 */

struct salink_label_t_ {
	uint8_t id;
	uint8_t type;
	union {
		uint16_t off;
		uint8_t equ[2];
	} data;
};
typedef struct salink_label_t_ salink_label_t;

struct salink_global_t_ {
	uint8_t obj_index;
	uint16_t line_no;
	uint16_t label_index;
	char name[SPECASM_LINE_MAX_LEN + 1];
};

typedef struct salink_global_t_ salink_global_t;

struct salink_obj_t_ {
	char fname[MAX_FNAME + 1];
	uint16_t label_start;
	uint16_t label_end;
	uint16_t size;
};
typedef struct salink_obj_t_ salink_obj_t;

extern char scratch[SPECASM_MAX_SCRATCH];
extern salink_label_t labels[MAX_LABELS];
extern salink_global_t globals[MAX_GLOBALS];
extern char error_buf[(SPECASM_LINE_MAX_LEN * 3) + 1];
unsigned int salink_find_local_label_e(const char* str, int len,
				       salink_obj_t *obj);
unsigned int salink_find_global_label_e(const char* str,
					salink_obj_t *obj);

#endif
