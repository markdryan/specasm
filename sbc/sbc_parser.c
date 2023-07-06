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

#include <string.h>

#include "sbc_error.h"
#include "sbc_lexer.h"
#include "sbc_overlay.h"
#include "sbc_parser.h"


struct sbc_parser_nest_t {
	/*
	 * Tail of the current body.
	 */
	sbc_handle_t body;
	uint8_t keyword;
	sbc_big_handle_t id;
};
typedef struct sbc_parser_nest_t sbc_parser_nest_t;

#define SBC_PARSER_MAX_NEST 12

static sbc_parser_nest_t nesting[SBC_PARSER_MAX_NEST];
static int8_t nest_level = -1;

static sbc_handle_t sbc_statement_start = 0;

sbc_statement_t sbc_statements[SBC_MAX_STATEMENTS];
sbc_line_t sbc_lines[SBC_MAX_LINES];

sbc_handle_t sbc_lines_start;



static uint8_t prv_is_simple_op(uint8_t op)
{
	return (overlay.lex.tok.type == SBC_TOKEN_OPERATOR) &&
		(overlay.lex.tok.len == 1) &&
		(overlay.lex.lex_buf[overlay.lex.tok.ptr] == op);
}

static void prv_handle_assignment_e(sbc_handle_t prev, uint8_t keyword)
{
	uint8_t op;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];
	uint8_t *str = &overlay.lex.lex_buf[overlay.lex.tok.ptr];

	s->d.assignment.id = sbc_pool_add_string_e(str, overlay.lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	op = sbc_exp_map_op();
	if ((op != SBC_EXP_EQ) && (op != SBC_EXP_PLUSEQ) &&
	    (op != SBC_EXP_MINUSEQ)) {
		err_type = SBC_ERROR_EQ_EXPECTED;
		return;
	}

	s->d.assignment.op = op;
	s->type = keyword;
	s->next = SBC_MAX_STATEMENTS;
	s->d.exp = sbc_exp_parse_e();
}

static void prv_handle_while_e(void)
{
	sbc_statement_t *s;

	s = &sbc_statements[sbc_statement_start];
	s->type = SBC_KEYWORD_WHILE;
	s->d.exp = sbc_exp_parse_e();
	s->body = SBC_MAX_STATEMENTS;
	s->next = SBC_MAX_STATEMENTS;
}

static void prv_handle_print_e(void)
{
	sbc_statement_t *s;

	s = &sbc_statements[sbc_statement_start];
	s->type = SBC_KEYWORD_PRINT;
	s->d.exp = sbc_exp_parse_e();
	s->next = SBC_MAX_STATEMENTS;
}

static int8_t prv_handle_keyword_e(void)
{
	int8_t compound = 0;

	switch (overlay.lex.tok.tok.keyword) {
	case SBC_KEYWORD_WHILE:
		prv_handle_while_e();
		compound = 1;
		break;
	case SBC_KEYWORD_PRINT:
		prv_handle_print_e();
		break;
	case SBC_KEYWORD_ENDWHILE:
		compound = -1;
		sbc_lexer_get_token_e();
		break;
	default:
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		break;
	}

	return compound;
}

static void prv_match_end_compound_e(sbc_handle_t stmt)
{
	switch (sbc_statements[stmt].type) {
	case SBC_KEYWORD_WHILE:
		if (nesting[nest_level].keyword != SBC_KEYWORD_ENDWHILE) {
			err_type = SBC_ERROR_ENDWHILE_EXPECTED;
			return;
		}
		break;
	default:
		break;
	}

}

static void prv_build_tree_e(int8_t compound, sbc_handle_t stmt)
{
	sbc_handle_t ptr;

	if (nest_level == -1)
		return;

	if (compound < 0) {

		/*
		 * We need to check our end compound statement matches our
		 * being compound statement, e.g., WHILE/ENDWHILE.
		 */

		prv_match_end_compound_e(stmt);
		nest_level--;
	}

	if (nest_level == -1)
		return;

	/*
	 * Then we are in a compound statement and we need to append the
	 * statements in this line onto the end of that compound
	 * statement.
	 */

	ptr = nesting[nest_level].body;
	sbc_statements[ptr].body = stmt;
	nesting[nest_level].body = stmt;
	if (compound <= 0)
		return;


	nest_level++;

	/*
	 * If the statement we appended is itself a compound statement we
	 * need to create a new level in the tree.
	 */

	if (nest_level == SBC_PARSER_MAX_NEST) {
		err_type = SBC_ERROR_TOO_MUCH_NESTING;
		return;
	}

	nesting[nest_level].keyword = sbc_statements[stmt].type;
	nesting[nest_level].body = stmt;
}

static void prv_parse_line_e(void)
{
	sbc_token_type_t tok_type;
	sbc_handle_t first = SBC_MAX_STATEMENTS;
	sbc_handle_t stmt;
	uint8_t compound;

	if (sbc_lines_start == (SBC_MAX_LINES - 1)) {
		err_type = SBC_ERROR_PROG_TOO_BIG;
		return;
	}

	sbc_lines[sbc_lines_start].line_no = overlay.lex.tok.tok.line_no;
	do {
		compound = 0;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return;

		if (prv_is_simple_op(':'))
			continue;

		tok_type = overlay.lex.tok.type;
		switch (tok_type) {
		case SBC_TOKEN_IDENTIFIER:
			prv_handle_assignment_e(stmt, SBC_KEYWORD_ASSIGN);
			break;
		case SBC_TOKEN_KEYWORD:
			compound = prv_handle_keyword_e();
			break;
		default:
			err_type = SBC_ERROR_KEYWORD_EXPECTED;
			return;
		}
		if (err_type != SPECASM_ERROR_OK)
			return;

		if (first != SBC_MAX_STATEMENTS)
			sbc_statements[stmt].next = sbc_statement_start;
		else
			first = sbc_statement_start;

		if (nest_level != -1) {
			prv_build_tree_e(compound, stmt);
			if (err_type != SPECASM_ERROR_OK)
				return;
		}
		stmt = sbc_statement_start++;
	} while (prv_is_simple_op(':'));

	if ((overlay.lex.tok.type != SBC_TOKEN_LINE_LABEL) &&
	    (overlay.lex.tok.type != SBC_TOKEN_EOF)) {
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		return;
	}

	sbc_lines[sbc_lines_start].stmt = first;
	sbc_lines_start++;
}

void sbc_parse_file_e(const char *f)
{
	sbc_lexer_open_e(f);
	if (err_type != SPECASM_ERROR_OK)
		return;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		goto cleanup;

	do {
		if (overlay.lex.tok.type == SBC_TOKEN_EOF)
			break;

		if (overlay.lex.tok.type != SBC_TOKEN_LINE_LABEL) {
			err_type = SBC_ERROR_BAD_LABEL;
			break;
		}

		prv_parse_line_e();
		if (err_type != SPECASM_ERROR_OK)
			break;

	} while (1);

cleanup:
	sbc_lexer_close();
}
