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
	sbc_handle_t compound;
	/*
	 * Tail of the current body.
	 */
	sbc_handle_t tail;
	uint8_t keyword;
};
typedef struct sbc_parser_nest_t sbc_parser_nest_t;

#define SBC_PARSER_MAX_NEST 12

static sbc_parser_nest_t nesting[SBC_PARSER_MAX_NEST];
static int8_t nest_level = 0;

sbc_handle_t sbc_statement_start = 0;
sbc_statement_t sbc_statements[SBC_MAX_STATEMENTS];


static uint8_t prv_is_simple_op(uint8_t op)
{
	return (overlay.lex.tok.type == SBC_TOKEN_OPERATOR) &&
		(overlay.lex.tok.len == 1) &&
		(overlay.lex.lex_buf[overlay.lex.tok.ptr] == op);
}

static void prv_parse_exps_e(uint8_t num)
{
	uint8_t i;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.four_exps.e[0] = sbc_exp_parse_no_get_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	for (i = 1; i < num; i++) {
		if (!prv_is_simple_op(',')) {
			err_type = SBC_ERROR_COMMA_EXPECTED;
			return;
		}
		s->d.four_exps.e[i] = sbc_exp_parse_e();
		if (err_type != SPECASM_ERROR_OK)
			return;
	}
}

static void prv_handle_assignment_e(uint8_t keyword)
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
	s->d.assignment.exp = sbc_exp_parse_e();
	s->next = SBC_MAX_STATEMENTS;
}

static void prv_handle_case_e(void)
{
	uint8_t *str;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (overlay.lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		err_type = SBC_ERROR_ID_EXPECTED;
		return;
	}

	str = &overlay.lex.lex_buf[overlay.lex.tok.ptr];
	s->d.compound.id = sbc_pool_add_string_e(str, overlay.lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = SBC_MAX_STATEMENTS;
	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	if ((overlay.lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (overlay.lex.tok.tok.keyword != SBC_KEYWORD_OF)) {
		err_type = SBC_ERROR_OF_EXPECTED;
		return;
	}
	sbc_lexer_get_token_e();
}

static void prv_handle_gcol_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.gcol.e1 = sbc_exp_parse_e();
	s->d.gcol.count = 1;

	if (prv_is_simple_op(',')) {
		s->d.gcol.count++;
		s->d.gcol.e2 = sbc_exp_parse_e();
	}
}

static void prv_handle_goto_gosub_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	if (overlay.lex.tok.type != SBC_TOKEN_LINE_NUMBER) {
		err_type = SBC_ERROR_LINENO_EXPECTED;
		return;
	}
	s->d.line_no = overlay.lex.tok.tok.line_no;
	sbc_lexer_get_token_e();
}

static void prv_handle_for_e(void)
{
	sbc_handle_t node;
	sbc_handle_t new_node;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];
	uint8_t *str = &overlay.lex.lex_buf[overlay.lex.tok.ptr];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (overlay.lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		err_type = SBC_ERROR_ID_EXPECTED;
		return;
	}

	s->d.compound.id = sbc_pool_add_string_e(str, overlay.lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (!prv_is_simple_op('=')) {
		err_type = SBC_ERROR_EQ_EXPECTED;
		return;
	}

	node = sbc_exp_get_node_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.eh = node;
	exp_list[node].e = sbc_exp_parse_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	s->d.compound.body = SBC_MAX_STATEMENTS;

	if ((overlay.lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (overlay.lex.tok.tok.keyword != SBC_KEYWORD_TO)) {
		err_type = SBC_ERROR_TO_EXPECTED;
		return;
	}

	new_node = sbc_exp_get_node_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	exp_list[node].next = new_node;
	node = new_node;
	exp_list[node].e = sbc_exp_parse_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((overlay.lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (overlay.lex.tok.tok.keyword != SBC_KEYWORD_STEP))
		return;

	s->type = SBC_KEYWORD_FOR_STEP;

	new_node = sbc_exp_get_node_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	exp_list[node].next = new_node;
	node = new_node;
	exp_list[node].e = sbc_exp_parse_e();
}

static void prv_handle_next_e(void)
{
	sbc_statement_t *s;
	uint8_t *str = &overlay.lex.lex_buf[overlay.lex.tok.ptr];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	s = &sbc_statements[sbc_statement_start];
	if (overlay.lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		s->d.str = SBC_INVALID_BIG_HANDLE;
		return;
	}

	s->d.str = sbc_pool_add_string_e(str, overlay.lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;
	sbc_lexer_get_token_e();
}

static void prv_handle_print_e(void)
{
	sbc_statement_t *s;

	s = &sbc_statements[sbc_statement_start];
	s->d.exp = sbc_exp_parse_e();
}

static void prv_handle_rect_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((overlay.lex.tok.type == SBC_TOKEN_KEYWORD) &&
	    (overlay.lex.tok.tok.keyword == SBC_KEYWORD_FILL)) {
		s->type = SBC_KEYWORD_RECT_FILL;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return;
	}

	prv_parse_exps_e(4);
}

static void prv_handle_rem_e(void)
{
	sbc_token_t *t;
	uint8_t *str;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	t = &overlay.lex.tok;
	str = &overlay.lex.lex_buf[t->ptr];
	s->type = SBC_KEYWORD_REM;
	s->d.str = sbc_pool_add_string_e(str, t->len);
	if (err_type != SPECASM_ERROR_OK)
		return;
	sbc_lexer_get_token_e();
}

static void prv_handle_while_e(void)
{
	sbc_statement_t *s;

	s = &sbc_statements[sbc_statement_start];
	s->d.compound.eh = sbc_exp_parse_e();
	s->d.compound.body = SBC_MAX_STATEMENTS;
}

static sbc_handle_t prv_parse_node_list_e(void)
{
	sbc_handle_t node;
	sbc_handle_t first;
	sbc_handle_t new_node;

	node = sbc_exp_get_node_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	first = node;

	exp_list[node].e = sbc_exp_parse_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	while (prv_is_simple_op(',')) {
		new_node = sbc_exp_get_node_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		exp_list[node].next = new_node;
		exp_list[new_node].e = sbc_exp_parse_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		node = new_node;
	}
	exp_list[node].next = SBC_MAX_EXP_NODES;

	return first;
}

static void prv_handle_when_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.compound.eh = prv_parse_node_list_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = SBC_MAX_STATEMENTS;
}

static int8_t prv_handle_keyword_e(void)
{
	uint8_t type;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];
	int8_t compound = 0;

	type = overlay.lex.tok.tok.keyword;

	/*
	 * We set this here so it can be overridden, e.g., RECTANGLE FILL
	 */

	s->type = type;
	switch (type) {
	case SBC_KEYWORD_CASE:
		prv_handle_case_e();
		compound = 1;
		break;
	case SBC_KEYWORD_ENDCASE:
		compound = -1;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		break;
	case SBC_KEYWORD_ENDWHILE:
		compound = -1;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		break;
	case SBC_KEYWORD_FOR:
		prv_handle_for_e();
		compound = 1;
		break;
	case SBC_KEYWORD_GCOL:
		prv_handle_gcol_e();
		break;
	case SBC_KEYWORD_GOTO:
	case SBC_KEYWORD_GOSUB:
		prv_handle_goto_gosub_e();
		break;
	case SBC_KEYWORD_END:
	case SBC_KEYWORD_OFF:
	case SBC_KEYWORD_ON:
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		break;
	case SBC_KEYWORD_MODE:
		s->d.exp = sbc_exp_parse_e();
		break;
	case SBC_KEYWORD_POINT2:
		s->d.four_exps.e[0] = sbc_exp_parse_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		s->d.four_exps.e[1] = sbc_exp_parse_e();
		break;
	case SBC_KEYWORD_NEXT:
		prv_handle_next_e();
		compound = -1;
		break;
	case SBC_KEYWORD_PRINT:
		prv_handle_print_e();
		break;
	case SBC_KEYWORD_RECT:
		prv_handle_rect_e();
		break;
	case SBC_KEYWORD_WHEN:
		prv_handle_when_e();
		compound = 1;
		break;
	case SBC_KEYWORD_WHILE:
		prv_handle_while_e();
		compound = 1;
		break;
	default:
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		break;
	}

	s->next = SBC_MAX_STATEMENTS;

	return compound;
}

/*
 * This is not quite right as it does not account for continuing a loop
 * of a parent compound loop.
 */

static void prv_match_end_compound_e(sbc_handle_t stmt)
{
	sbc_big_handle_t id;

	switch (sbc_statements[stmt].type) {
	case SBC_KEYWORD_ENDWHILE:
		if (nesting[nest_level].keyword != SBC_KEYWORD_WHILE) {
			err_type = SBC_ERROR_ENDWHILE_UNEXPECTED;
			return;
		}
		break;
	case SBC_KEYWORD_NEXT:
		if ((nesting[nest_level].keyword != SBC_KEYWORD_FOR) &&
		    (nesting[nest_level].keyword != SBC_KEYWORD_FOR_STEP)) {
			err_type = SBC_ERROR_NEXT_UNEXPECTED;
			return;
		}
		id = sbc_statements[stmt].d.str;
		if (id == SBC_INVALID_BIG_HANDLE)
			return;
		if (id ==
		    sbc_statements[nesting[nest_level].compound].d.compound.id)
			return;
		err_type = SBC_ERROR_NEXT_UNEXPECTED;
		break;
	case SBC_KEYWORD_ENDCASE:
		if (nesting[nest_level].keyword != SBC_KEYWORD_CASE) {
			err_type = SBC_ERROR_ENDCASE_EXPECTED;
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
	sbc_parser_nest_t *nest;

	if (compound < 0) {

		/*
		 * We need to check our end compound statement matches our
		 * begin compound statement, e.g., WHILE/ENDWHILE.
		 */

		prv_match_end_compound_e(stmt);
		if (err_type != SPECASM_ERROR_OK)
			return;
		nest_level--;
	}

	/*
	 * If this is the first statement of a compound statement we need
	 * to update the body pointer in the compound statement.
	 */

	nest = &nesting[nest_level];
	ptr = nest->tail;
	if (ptr == SBC_MAX_STATEMENTS)
		sbc_statements[nest->compound].d.compound.body = stmt;
	else
		sbc_statements[ptr].next = stmt;
	nest->tail = stmt;
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

	nest = &nesting[nest_level];
	nest->keyword = sbc_statements[stmt].type;
	nest->compound = stmt;
	nest->tail = SBC_MAX_STATEMENTS;
}

static void prv_parse_line_e(void)
{
	sbc_token_type_t tok_type;
	uint8_t compound;
	uint16_t line_no;

	line_no = overlay.lex.tok.tok.line_no;
	do {
		compound = 0;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return;

		sbc_statements[sbc_statement_start].line_no = line_no;

		tok_type = overlay.lex.tok.type;

		if (tok_type == SBC_TOKEN_EOF)
			break;

		switch (tok_type) {
		case SBC_TOKEN_IDENTIFIER:
			prv_handle_assignment_e(SBC_KEYWORD_ASSIGN);
			break;
		case SBC_TOKEN_KEYWORD:
			compound = prv_handle_keyword_e();
			break;
		case SBC_TOKEN_REM:
			prv_handle_rem_e();
			break;
		case SBC_TOKEN_OPERATOR:
			if (prv_is_simple_op(':')) {
				sbc_statements[sbc_statement_start].type =
					SBC_KEYWORD_BLANK;
				sbc_lexer_get_token_e();
				if (err_type != SPECASM_ERROR_OK)
					return;
			} else {
				err_type = SBC_ERROR_KEYWORD_EXPECTED;
				return;
			}
			break;
		default:
			err_type = SBC_ERROR_KEYWORD_EXPECTED;
			return;
		}
		if (err_type != SPECASM_ERROR_OK)
			return;

		prv_build_tree_e(compound, sbc_statement_start);
		if (err_type != SPECASM_ERROR_OK)
			return;

		sbc_statement_start++;
	} while (prv_is_simple_op(':'));

	if ((overlay.lex.tok.type != SBC_TOKEN_LINE_LABEL) &&
	    (overlay.lex.tok.type != SBC_TOKEN_EOF)) {
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		return;
	}

	/*
	 * Check for end of compound statements that do not have a terminating
	 * keyword.
	 */

	if ((nest_level > 0) &&
	    (nesting[nest_level].keyword == SBC_KEYWORD_WHEN))
		nest_level--;
}

void sbc_parse_file_e(const char *f)
{
	sbc_lexer_open_e(f);
	if (err_type != SPECASM_ERROR_OK)
		return;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		goto cleanup;

	nesting[0].tail = SBC_MAX_STATEMENTS;
	nesting[0].keyword = SBC_KEYWORD_INVALID;

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
