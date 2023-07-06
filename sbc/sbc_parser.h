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

struct sbc_assignment_t {
	uint8_t op;
	sbc_big_handle_t id;
	sbc_handle_t exp;
};
typedef struct sbc_assignment_t sbc_assignment_t;

struct sbc_four_exps_t {
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_handle_t e3;
	sbc_handle_t e4;
};
typedef struct sbc_four_exps_t sbc_four_exps_t;

struct sbc_statement_t {
	uint8_t type;
	union {
		sbc_assignment_t assignment;
		sbc_expression_node_t exp_list;
		sbc_handle_t exp;
		sbc_four_exps_t four_exps;
	} d;

	/*
	 * Points to next statement in this compound statement.
	 */
	sbc_handle_t body;

	/*
	 * Points to next statement on this line.
	 */
	sbc_handle_t next;
};
typedef struct sbc_statement_t sbc_statement_t;

/*
 * If stmt = SBC_MAX_STATEMENTS it's a ':' or a NOP.  This allows
 * us to have blank lines in the code for formatting without using
 * one of our precious statements.
 */

struct sbc_line_t {
	uint16_t line_no;
	sbc_handle_t stmt;
};
typedef struct sbc_line_t sbc_line_t;

/*
 * How much memory is all this going to take.  On the Spectrum we have
 *
 * string pool     =            1024
 * expressions     =  256 * 6 = 1536
 * expression_list =  256 * 2 =  512
 * statements =    =  255 * 7 = 1785
 * lines           =  256 * 3 =  768
 *
 * Total                      = 5625
 */

#define SBC_MAX_STATEMENTS ((256 * SBC_CONFIG_SIZE) - 1)
#define SBC_MAX_LINES (256 * SBC_CONFIG_SIZE)

extern sbc_statement_t sbc_statements[SBC_MAX_STATEMENTS];
extern sbc_line_t sbc_lines[SBC_MAX_LINES];

extern sbc_handle_t sbc_lines_start;

void sbc_parse_file_e(const char *f);

#endif
