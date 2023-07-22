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

#ifndef SBC_PARSER_H
#define SBC_PARSER_H

#include "sbc_config.h"
#include "sbc_exp.h"

#define SBC_KEYWORD_ASSIGN SBC_KEYWORD_UNUSED
#define SBC_KEYWORD_BLANK  (SBC_KEYWORD_UNUSED + 1)
#define SBC_KEYWORD_RECT_FILL  (SBC_KEYWORD_UNUSED + 2)
#define SBC_KEYWORD_FOR_STEP  (SBC_KEYWORD_UNUSED + 3)
#define SBC_KEYWORD_ELSE_THEN  (SBC_KEYWORD_UNUSED + 4)
#define SBC_KEYWORD_DEF_PROC  (SBC_KEYWORD_UNUSED + 5)
#define SBC_KEYWORD_DEF_FN  (SBC_KEYWORD_UNUSED + 6)

struct sbc_assignment_t {
	uint8_t op;
	sbc_big_handle_t id;
	sbc_handle_t exp;
};
typedef struct sbc_assignment_t sbc_assignment_t;

struct sbc_compound_t {
	sbc_big_handle_t id;

	/*
	 * Either a pointer to an expression or for a for
	 * an expression node.
	 */
	sbc_handle_t eh;
	sbc_handle_t body;
};
typedef struct sbc_compound_t sbc_compound_t;

struct sbc_four_exps_t {
	sbc_handle_t e[4];
};
typedef struct sbc_four_exps_t sbc_four_exps_t;

struct sbc_gcol_exps_t {
	uint8_t count;
	sbc_handle_t e1;
	sbc_handle_t e2;
};
typedef struct sbc_gcol_exps_t sbc_gcol_exps_t;

struct sbc_statement_t {
	uint8_t type;
	union {
		sbc_assignment_t assignment;
		sbc_compound_t compound;
		sbc_handle_t exp_list;
		sbc_handle_t exp;
		sbc_four_exps_t four_exps;
		uint16_t line_no;
		sbc_big_handle_t str;
		sbc_gcol_exps_t gcol;
	} d;

	/*
	 * Points to next statement in this compound statement.
	 */
	sbc_handle_t next;
	uint16_t line_no;
};
typedef struct sbc_statement_t sbc_statement_t;


/*
 * How much memory is all this going to take.  On the Spectrum we have
 *
 * string pool     =            1024
 * expressions     =  256 * 6 = 1536
 * expression_list =  255 * 2 =  510
 * statements =    =  255 * 8 = 2040
 *
 * Total                      = 5110
 */

#define SBC_MAX_STATEMENTS ((256 * SBC_CONFIG_SIZE) - 1)

extern sbc_statement_t sbc_statements[SBC_MAX_STATEMENTS];
extern sbc_handle_t sbc_statement_start;

void sbc_parse_file_e(const char *f);

#endif
