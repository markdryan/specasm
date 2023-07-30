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

#include "sbc_parser.h"


struct sbc_parser_nest_t {
	sbc_handle_t compound;
	/*
	 * Tail of the current body.
	 */
	sbc_handle_t tail;
	uint8_t keyword;
	uint16_t line_no;
};
typedef struct sbc_parser_nest_t sbc_parser_nest_t;

#define SBC_PARSER_MAX_NEST 12

static sbc_parser_nest_t nesting[SBC_PARSER_MAX_NEST];
static int8_t nest_level = 0;

sbc_handle_t sbc_statement_start = 0;
sbc_statement_t sbc_statements[SBC_MAX_STATEMENTS];

static void prv_handle_proc_fn_e(void);

static void prv_parse_exps_e(uint8_t num)
{
	uint8_t i;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.four_exps.e[0] = sbc_exp_parse_no_get_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	for (i = 1; i < num; i++) {
		if (!sbc_exp_is_simple_op(',')) {
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
	uint8_t *str = &lex.lex_buf[lex.tok.ptr];

	s->d.assignment.id = sbc_pool_add_string_e(str, lex.tok.len);
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

	if (lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		err_type = SBC_ERROR_ID_EXPECTED;
		return;
	}

	str = &lex.lex_buf[lex.tok.ptr];
	s->d.compound.id = sbc_pool_add_string_e(str, lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = SBC_MAX_STATEMENTS;
	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	if ((lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (lex.tok.tok.keyword != SBC_KEYWORD_OF)) {
		err_type = SBC_ERROR_OF_EXPECTED;
		return;
	}
	sbc_lexer_get_token_e();
}

static void prv_handle_def_e(void)
{
	sbc_handle_t eh;
	sbc_expression_t *e;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    ((lex.tok.tok.keyword != SBC_KEYWORD_PROC) &&
	     (lex.tok.tok.keyword != SBC_KEYWORD_FN))) {
		err_type = SBC_ERROR_PROC_OR_FN_EXPECTED;
		return;
	}

	s->type = lex.tok.tok.keyword == SBC_KEYWORD_PROC ?
		SBC_KEYWORD_DEF_PROC : SBC_KEYWORD_DEF_FN;

	prv_handle_proc_fn_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = SBC_MAX_STATEMENTS;

	/*
	 * We parse a procedure/function defintion as if it was a call to avoid
	 * having to maintain a list of identifiers and the code to parse such
	 * lists.  Thus we need to iterate through all the formal parameters and
	 * check that they are identifiers.  We should probably check that there
	 * are no duplicate parameters as well.
	 */

	/*
	 * TODO check for duplicates.
	 */

	eh = s->d.compound.eh;
	while (eh != SBC_MAX_EXP_NODES) {
		e = &sbc_expressions[exp_list[eh].e];
		if (e->type != SBC_EXP_IDENTIFIER) {
			err_type = SBC_ERROR_ID_EXPECTED;
			return;
		}
		eh = exp_list[eh].next;
	}
}

static void prv_handle_gcol_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.gcol.e1 = sbc_exp_parse_e();
	s->d.gcol.count = 1;

	if (sbc_exp_is_simple_op(',')) {
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
	if (lex.tok.type != SBC_TOKEN_LINE_NUMBER) {
		err_type = SBC_ERROR_LINENO_EXPECTED;
		return;
	}
	s->d.line_no = lex.tok.tok.line_no;
	sbc_lexer_get_token_e();
}

static void prv_handle_for_e(void)
{
	sbc_handle_t node;
	sbc_handle_t new_node;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];
	uint8_t *str = &lex.lex_buf[lex.tok.ptr];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		err_type = SBC_ERROR_ID_EXPECTED;
		return;
	}

	s->d.compound.id = sbc_pool_add_string_e(str, lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (!sbc_exp_is_simple_op('=')) {
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

	if ((lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (lex.tok.tok.keyword != SBC_KEYWORD_TO)) {
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

	if ((lex.tok.type != SBC_TOKEN_KEYWORD) ||
	    (lex.tok.tok.keyword != SBC_KEYWORD_STEP))
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
	uint8_t *str = &lex.lex_buf[lex.tok.ptr];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	s = &sbc_statements[sbc_statement_start];
	if (lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		s->d.str = SBC_INVALID_BIG_HANDLE;
		return;
	}

	s->d.str = sbc_pool_add_string_e(str, lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;
	sbc_lexer_get_token_e();
}

static void prv_handle_if_e(void)
{
	sbc_statement_t *s;

	s = &sbc_statements[sbc_statement_start];
	s->d.compound.eh = sbc_exp_parse_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	/*
	 * For multiline if statements we'll use the type of THEN
	 * rather than IF to distinguish between the two.
	 */

	if ((lex.tok.type == SBC_TOKEN_KEYWORD) &&
	    (lex.tok.tok.keyword == SBC_KEYWORD_THEN)) {
		s->type = SBC_KEYWORD_THEN;
		sbc_lexer_get_token_e();
	}
	s->d.compound.body = SBC_MAX_STATEMENTS;
}

static sbc_handle_t prv_parse_print_command_e(uint16_t line_no)
{
	sbc_statement_t *s;

	if (lex.tok.type == SBC_TOKEN_LINE_LABEL)
		return SBC_MAX_STATEMENTS;

	if (sbc_statement_start == SBC_MAX_STATEMENTS) {
		err_type = SBC_ERROR_TOO_MANY_STATEMENTS;
		return SBC_MAX_STATEMENTS;
	}
	sbc_statement_start++;
	s = &sbc_statements[sbc_statement_start];
	s->line_no = line_no;
	s->next = SBC_MAX_STATEMENTS;

	switch (lex.tok.type) {
	case SBC_TOKEN_KEYWORD:
		if (lex.tok.tok.keyword != SBC_KEYWORD_TAB) {
			s->type = SBC_KEYWORD_PRINT_SPACE;
			s->d.exp = sbc_exp_parse_no_get_e();
			break;
		}
		s->type = SBC_KEYWORD_TAB;
		s->d.gcol.e1 = sbc_exp_parse_e();
		if (err_type != SPECASM_ERROR_OK)
			return SBC_MAX_STATEMENTS;
		s->d.gcol.count = 1;
		if (sbc_exp_is_simple_op(',')) {
			s->d.gcol.e2 = sbc_exp_parse_e();
			if (err_type != SPECASM_ERROR_OK)
				return SBC_MAX_STATEMENTS;
			s->d.gcol.count++;
			if (!sbc_exp_is_simple_op(')')) {
				err_type = SBC_ERROR_CLOSEB_EXPECTED;
				return SBC_MAX_STATEMENTS;
			}
			sbc_lexer_get_token_e();
		}
		break;
	case SBC_TOKEN_OPERATOR:
		s->d.exp = SBC_MAX_EXPRESSIONS;
		switch (sbc_exp_map_op()) {
		case ';':
			s->type = SBC_KEYWORD_PRINT_SEMIC;
			break;
		case '\'':
			s->type = SBC_KEYWORD_PRINT_QUOTE;
			break;
		case ',':
			s->type = SBC_KEYWORD_PRINT_COMMA;
			break;
		default:
			err_type = SBC_ERROR_SYNTAX_ERROR;
			return SBC_MAX_STATEMENTS;
		}
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return SBC_MAX_STATEMENTS;
		if ((lex.tok.type == SBC_TOKEN_LINE_LABEL) ||
		    (lex.tok.type == SBC_TOKEN_OPERATOR))
			break;
		s->d.exp = sbc_exp_parse_no_get_e();
		break;
	default:
		s->type = SBC_KEYWORD_PRINT_SPACE;
		s->d.exp = sbc_exp_parse_no_get_e();
		break;
	}

	return sbc_statement_start;
}

static void prv_handle_print_e(void)
{
	sbc_statement_t *s;
	sbc_handle_t h;

	/*
	 * We're going to treat PRINT as a compound statement.
	 * but we're going to build up the statement manually
	 * in this function instead of in build_tree.  We can do this
	 * as the entire PRINT statement is restricted to one line.
	 * Having it as one compound statement makes it easier to format.
	 */

	s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	h = prv_parse_print_command_e(s->line_no);
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = h;
	while (h != SBC_MAX_STATEMENTS) {
		s = &sbc_statements[h];
		h = prv_parse_print_command_e(s->line_no);
		if (err_type != SPECASM_ERROR_OK)
			return;
		s->next =  h;
	}
}

static void prv_handle_proc_fn_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	s->d.compound.id = sbc_pool_add_string_e(&lex.lex_buf[lex.tok.ptr],
						 lex.tok.len);
	if (err_type != SPECASM_ERROR_OK)
		return;

	s->d.compound.eh = sbc_parse_bracketednode_list_e();
}

static void prv_handle_rect_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((lex.tok.type == SBC_TOKEN_KEYWORD) &&
	    (lex.tok.tok.keyword == SBC_KEYWORD_FILL)) {
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

	t = &lex.tok;
	str = &lex.lex_buf[t->ptr];
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

static void prv_handle_when_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->d.compound.eh = sbc_parse_node_list_e();
	if (err_type != SPECASM_ERROR_OK)
		return;
	s->d.compound.body = SBC_MAX_STATEMENTS;
}

static int8_t prv_handle_keyword_e(void)
{
	uint8_t type;
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];
	int8_t compound = 0;

	type = lex.tok.tok.keyword;

	/*
	 * We set this here so it can be overridden, e.g., RECTANGLE FILL
	 */

	s->type = type;
	switch (type) {
	case SBC_KEYWORD_CASE:
		prv_handle_case_e();
		compound = 1;
		break;
	case SBC_KEYWORD_DEF:
		prv_handle_def_e();
		compound = 1;
		break;
	case SBC_KEYWORD_ENDCASE:
	case SBC_KEYWORD_ENDIF:
	case SBC_KEYWORD_ENDWHILE:
	case SBC_KEYWORD_ENDPROC:
		compound = -1;
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		break;
	case SBC_KEYWORD_FOR:
		prv_handle_for_e();
		compound = 1;
		break;
	case SBC_KEYWORD_PROC:
		prv_handle_proc_fn_e();
		break;
	case SBC_KEYWORD_GCOL:
		prv_handle_gcol_e();
		break;
	case SBC_KEYWORD_GOTO:
	case SBC_KEYWORD_GOSUB:
		prv_handle_goto_gosub_e();
		break;
	case SBC_KEYWORD_IF:
		prv_handle_if_e();
		compound = 1;
		break;
	case SBC_KEYWORD_ELSE:
		if (nesting[nest_level].keyword == SBC_KEYWORD_THEN)
			s->type = SBC_KEYWORD_ELSE_THEN;
		s->d.compound.body = SBC_MAX_STATEMENTS;
		compound = 1;
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
	case SBC_KEYWORD_MOVE:
	case SBC_KEYWORD_DRAW:
	case SBC_KEYWORD_ORIGIN:
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		prv_parse_exps_e(2);
		break;
	case SBC_KEYWORD_PLOT:
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		prv_parse_exps_e(3);
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
	case SBC_KEYWORD_VDU:
		/*
		 * This isn't correct.  VDU has a special syntax.
		 */
		s->d.exp_list = sbc_parse_node_list_e();
		break;
	default:
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		break;
	}

	s->next = SBC_MAX_STATEMENTS;

	return compound;
}

static int8_t prv_check_end_compound_e(uint8_t keyword, specasm_error_t err)
{
	int8_t i;

	/*
	 * There must be a keyword somewhere in the nesting stack.
	 */

	if (nesting[nest_level].keyword == keyword)
		return -1;

	for (i = nest_level - 1; i >= 0; i--)
		if (nesting[i].keyword == keyword)
			return 0;
	err_type = err;

	return 0;
}

static int8_t prv_match_end_compound_e(sbc_handle_t stmt)
{
	sbc_big_handle_t id;
	int8_t compound = -1;

	switch (sbc_statements[stmt].type) {
	case SBC_KEYWORD_ENDWHILE:
		compound = prv_check_end_compound_e(
			SBC_KEYWORD_WHILE, SBC_ERROR_ENDWHILE_UNEXPECTED);
		break;
	case SBC_KEYWORD_NEXT:
		compound = prv_check_end_compound_e(SBC_KEYWORD_FOR,
						    SBC_ERROR_NEXT_UNEXPECTED);
		if (err_type != SPECASM_ERROR_OK) {
			err_type = SPECASM_ERROR_OK;
			compound = prv_check_end_compound_e(
				SBC_KEYWORD_FOR_STEP,
				SBC_ERROR_NEXT_UNEXPECTED);
		}
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		id = sbc_statements[stmt].d.str;
		if (id == SBC_INVALID_BIG_HANDLE)
			return compound;
		if (id ==
		    sbc_statements[nesting[nest_level].compound].d.compound.id)
			return compound;
		err_type = SBC_ERROR_NEXT_UNEXPECTED;
		break;
	case SBC_KEYWORD_ENDCASE:
		compound = prv_check_end_compound_e(
			SBC_KEYWORD_CASE, SBC_ERROR_ENDCASE_UNEXPECTED);
		break;
	case SBC_KEYWORD_ENDPROC:
		compound = prv_check_end_compound_e(
			SBC_KEYWORD_DEF_PROC, SBC_ERROR_ENDPROC_UNEXPECTED);
		break;
	case SBC_KEYWORD_ENDIF:
	case SBC_KEYWORD_ELSE:
	case SBC_KEYWORD_ELSE_THEN:
		compound = prv_check_end_compound_e(
			SBC_KEYWORD_THEN, SBC_ERROR_ELSE_OR_ENDIF_UNEXPECTED);
		if (err_type != SPECASM_ERROR_OK) {
			err_type = SPECASM_ERROR_OK;
			compound = prv_check_end_compound_e(
				SBC_KEYWORD_IF,
				SBC_ERROR_ELSE_OR_ENDIF_UNEXPECTED);
		}
		break;
	default:
		break;
	}

	return compound;
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

		compound = prv_match_end_compound_e(stmt);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (compound < 0)
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
	nest->line_no = sbc_statements[stmt].line_no;
}

static uint8_t prv_handle_end_fn_e(void)
{
	sbc_statement_t *s = &sbc_statements[sbc_statement_start];

	s->type = SBC_KEYWORD_END_FN;
	s->next = SBC_MAX_STATEMENTS;
	s->d.exp = sbc_exp_parse_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return prv_check_end_compound_e(SBC_KEYWORD_DEF_FN,
					SBC_ERROR_EQ_UNEXPECTED);
}

static void prv_parse_line_e(void)
{
	sbc_token_type_t tok_type;
	uint8_t compound;
	uint16_t line_no;
	uint8_t nesting_kw;
	sbc_handle_t h;
	uint8_t cont = 0;

	line_no = lex.tok.tok.line_no;
	do {
		compound = 0;

		/*
		 * If the previous statement was an IF or a THEN then
		 * we've already done the sbc_lexer_get_token_e().
		 */

		if (!cont) {
			sbc_lexer_get_token_e();
			if (err_type != SPECASM_ERROR_OK)
				return;
		} else if (lex.tok.type == SBC_TOKEN_LINE_LABEL) {
			break;
		}

		cont = 0;

		tok_type = lex.tok.type;

		if (tok_type == SBC_TOKEN_EOF)
			break;

		sbc_statements[sbc_statement_start].line_no = line_no;
		if (tok_type == SBC_TOKEN_LINE_LABEL) {
			sbc_statements[sbc_statement_start++].type =
				SBC_KEYWORD_BLANK;
			break;
		}

		/*
		 * PRINT breaks the normal statement processing as it
		 * can create multiple statements.  So we need to save
		 * sbc_statement_start here so we know which statement
		 * to add to the tree.
		 */

		h = sbc_statement_start;
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
			if (sbc_exp_is_simple_op(':')) {
				sbc_statements[sbc_statement_start].type =
					SBC_KEYWORD_BLANK;
				sbc_lexer_get_token_e();
			} else if (sbc_exp_is_simple_op('=')) {
				compound = prv_handle_end_fn_e();
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

		prv_build_tree_e(compound, h);
		if (err_type != SPECASM_ERROR_OK)
			return;

		nesting_kw = nesting[nest_level].keyword;
		cont = (compound > 0) &&
			((nesting_kw == SBC_KEYWORD_THEN) ||
			 (nesting_kw == SBC_KEYWORD_IF));

		sbc_statement_start++;
	} while (sbc_exp_is_simple_op(':') || cont);

	if ((lex.tok.type != SBC_TOKEN_LINE_LABEL) &&
	    (lex.tok.type != SBC_TOKEN_EOF)) {
		err_type = SBC_ERROR_KEYWORD_EXPECTED;
		return;
	}

	/*
	 * Check for end of compound statements that do not have a terminating
	 * keyword.
	 */

	if (nest_level > 0) {
		if ((nesting[nest_level].keyword == SBC_KEYWORD_WHEN) ||
		    (nesting[nest_level].keyword == SBC_KEYWORD_IF) ||
		    (nesting[nest_level].keyword == SBC_KEYWORD_ELSE))
			nest_level--;

		/*
		 * It's only a compound if statement if the THEN is the last
		 * keyword on the line.
		 */

		if ((nesting[nest_level].keyword == SBC_KEYWORD_THEN) &&
		    (nesting[nest_level].line_no ==
		     sbc_statements[sbc_statement_start - 1].line_no) &&
		    (sbc_statements[sbc_statement_start - 1].type !=
		     SBC_KEYWORD_THEN))
			nest_level--;
	}
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
	nesting[0].line_no = 0;

	do {
		if (lex.tok.type == SBC_TOKEN_EOF)
			break;

		if (lex.tok.type != SBC_TOKEN_LINE_LABEL) {
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
