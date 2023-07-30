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

#include "sbc_config.h"
#include "sbc_error.h"
#include "sbc_exp.h"


/*
 * Strings have a 1 byte length followed by the string contents.
 * Last string is null terminated.
 */

uint8_t sbc_pool_strings[SBC_POOL_MAX_STRING_BUF + 1];

static sbc_handle_t sbc_exp_start = 0;
static sbc_handle_t sbc_exp_end = (sbc_handle_t) (SBC_MAX_EXPRESSIONS - 1);
static sbc_handle_t sbc_exp_list_start;

/*
 * We'll store leaf expressions at the start and compound expressions at the
 * end.  That may make searching a little quicker.
 */

sbc_expression_t sbc_expressions[SBC_MAX_EXPRESSIONS];
sbc_expression_node_t exp_list[SBC_MAX_EXP_NODES];

uint8_t sbc_exp_is_simple_op(uint8_t op)
{
	return (lex.tok.type == SBC_TOKEN_OPERATOR) &&
		(lex.tok.len == 1) &&
		(lex.lex_buf[lex.tok.ptr] == op);
}

sbc_handle_t sbc_exp_get_node_e(void)
{
	if (sbc_exp_list_start == (SBC_MAX_EXP_NODES) - 1) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return SBC_MAX_EXP_NODES - 1;
	}
	return sbc_exp_list_start++;
}

sbc_handle_t sbc_parse_node_list_e(void)
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

	while (sbc_exp_is_simple_op(',')) {
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

sbc_handle_t sbc_parse_bracketednode_list_e(void)
{
	sbc_handle_t h;

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (!sbc_exp_is_simple_op('(')) {
		return SBC_MAX_EXP_NODES;
	}

	h = sbc_parse_node_list_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (!sbc_exp_is_simple_op(')')) {
		err_type = SBC_ERROR_CLOSEB_EXPECTED;
		return 0;
	}

	sbc_lexer_get_token_e();

	return h;
}

sbc_big_handle_t sbc_pool_add_string_e(const uint8_t *v, uint8_t len)
{
	sbc_big_handle_t start;
	uint8_t *ptr = &sbc_pool_strings[0];

	while (*ptr) {
		if (*ptr == len) {
			if (!memcmp(ptr + 1, v, len))
				return ptr - &sbc_pool_strings[0];
		}
		ptr += *ptr + 1;
	}

	start = ptr - &sbc_pool_strings[0];
	if (start + len + 1 > SBC_POOL_MAX_STRING_BUF) {
		err_type = SBC_ERROR_TOO_MANY_STRINGS;
		return 0;
	}
	*ptr++ = len;
	memcpy(ptr, v, len);
	ptr[len] = 0;

	return start;
}

sbc_handle_t sbc_exp_add_int_e(sbc_token_t *t)
{
	sbc_handle_t h;
	sbc_expression_t *e;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	for (h = 0; h < sbc_exp_start; h++) {
		e = &sbc_expressions[h];
		if ((e->type == t->type) && (e->v.integer == t->tok.integer))
			return h;
	}
	e = &sbc_expressions[h];
	sbc_exp_start++;
	e->type = t->type;
	e->v.integer = t->tok.integer;

	return h;
}

sbc_handle_t sbc_exp_add_real_e(sbc_token_t *t)
{
	sbc_handle_t h;
	sbc_expression_t *e;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	for (h = 0; h < sbc_exp_start; h++) {
		e = &sbc_expressions[h];
		if ((e->type == t->type) &&
		    !memcmp(&e->v.real, &t->tok.real, sizeof(e->v.real)))
			return h;
	}
	e = &sbc_expressions[h];
	sbc_exp_start++;
	e->type = t->type;
	memcpy(&e->v.real, &t->tok.real, sizeof(e->v.real));

	return h;
}

sbc_handle_t sbc_exp_add_id_base_e(sbc_token_t *t, uint8_t id_type)
{
	sbc_handle_t h;
	sbc_big_handle_t sh;
	sbc_expression_t *e;
	uint8_t *str = &lex.lex_buf[t->ptr];

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	sh = sbc_pool_add_string_e(str, t->len);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	for (h = 0; h < sbc_exp_start; h++) {
		e = &sbc_expressions[h];
		if ((e->type == t->type) && (sh == e->v.id.str))
			return h;
	}

	e = &sbc_expressions[h];
	sbc_exp_start++;
	e->type = t->type;
	e->v.id.str = sh;
	e->v.id.id_type = id_type;

	return h;
}

uint8_t sbc_exp_map_op(void)
{
	char b1;
	char b2;

	if (lex.tok.len == 1)
		return lex.lex_buf[lex.tok.ptr];
	if (lex.tok.len == 3)
		return SBC_EXP_ASR;
	b1 = (char) lex.lex_buf[lex.tok.ptr];
	b2 = (char) lex.lex_buf[lex.tok.ptr + 1];

	if (b1 == '-')
		return SBC_EXP_MINUSEQ;

	if (b1 == '+')
		return SBC_EXP_PLUSEQ;

	if (b1 == '<') {
		if (b2 == '>')
			return SBC_EXP_NEQ;
		else if (b2 == '<')
			return SBC_EXP_LSL;
		return SBC_EXP_LTE;
	}

	if (b2 == '>')
		return SBC_EXP_LSR;

	return SBC_EXP_GTE;
}

static sbc_handle_t prv_no_arg_keyword(uint8_t op)
{
	sbc_expression_t *e;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	e = &sbc_expressions[sbc_exp_start];
	e->type = op;

	return sbc_exp_start++;
}


static sbc_handle_t prv_single_arg_fn(uint8_t op)
{
	sbc_expression_t *e;
	sbc_handle_t h;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	h = sbc_exp_parse_e();
	e = &sbc_expressions[sbc_exp_start];
	e->type = op;
	e->v.args.a1 = h;

	return sbc_exp_start++;
}

static sbc_handle_t prv_add_fn(void)
{
	sbc_expression_t *e;
	sbc_token_t *t = &lex.tok;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (lex.tok.type != SBC_TOKEN_IDENTIFIER) {
		err_type = SBC_ERROR_ID_EXPECTED;
		return 0;
	}

	e = &sbc_expressions[sbc_exp_start];
	e->type = SBC_EXP_FN;
	e->v.fn_call.name = sbc_pool_add_string_e(&lex.lex_buf[t->ptr], t->len);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	e->v.fn_call.args_list = sbc_parse_bracketednode_list_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return sbc_exp_start++;
}

static sbc_handle_t prv_priority1_e(void)
{
	sbc_token_type_t tok_type;
	sbc_handle_t h = 0;
	sbc_expression_t *e;

	tok_type = lex.tok.type;
	switch (tok_type) {
	case SBC_TOKEN_INTEGER:
	case SBC_TOKEN_HEX:
	case SBC_TOKEN_BIN:
		h = sbc_exp_add_int_e(&lex.tok);
		break;
	case SBC_TOKEN_REAL:
		h = sbc_exp_add_real_e(&lex.tok);
		break;
	case SBC_TOKEN_IDENTIFIER:
		h = sbc_exp_add_id_e(&lex.tok);
		break;
	case SBC_TOKEN_STRING:
		h = sbc_exp_add_string_e(&lex.tok);
		break;
	case SBC_TOKEN_OPERATOR:
		switch (sbc_exp_map_op()) {
		case SBC_EXP_OPENB:
			h = sbc_exp_parse_e();
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			if ((lex.tok.type != SBC_TOKEN_OPERATOR) ||
			    (sbc_exp_map_op() != SBC_EXP_CLOSEB)) {
				err_type = SBC_ERROR_CLOSEB_EXPECTED;
				return 0;
			}
		break;
		case '-':
			if (sbc_exp_start == sbc_exp_end) {
				err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
				return 0;
			}
			h = sbc_exp_start++;
			e = &sbc_expressions[h];
			e->type = SBC_EXP_UMINUS;
			e->v.args.a1 = sbc_exp_parse_e();
			return h;
		}
		break;
	case SBC_TOKEN_KEYWORD:
		switch (lex.tok.tok.keyword) {
		case SBC_KEYWORD_FN:
			return prv_add_fn();
		case SBC_KEYWORD_RAD:
			return prv_single_arg_fn(SBC_EXP_RAD);
		case SBC_KEYWORD_RND:
			return prv_single_arg_fn(SBC_EXP_RND);
		case SBC_KEYWORD_COS:
			return prv_single_arg_fn(SBC_EXP_COS);
		case SBC_KEYWORD_SIN:
			return prv_single_arg_fn(SBC_EXP_SIN);
		case SBC_KEYWORD_SQR:
			return prv_single_arg_fn(SBC_EXP_SQR);
		case SBC_KEYWORD_CHR_STR:
			return prv_single_arg_fn(SBC_EXP_CHR_STR);
		case SBC_KEYWORD_GET:
			h = prv_no_arg_keyword(SBC_EXP_GET);
			break;
		case SBC_KEYWORD_TIME:
			h = prv_no_arg_keyword(SBC_EXP_TIME);
			break;
		case SBC_KEYWORD_PI:
			h = prv_no_arg_keyword(SBC_EXP_PI);
			break;
		default:
			err_type = SBC_ERROR_EXP_EXPECTED;
			break;
		}
		break;
	default:
		err_type = SBC_ERROR_EXP_EXPECTED;
		break;
	}

	if (err_type != SPECASM_ERROR_OK)
		return 0;

	sbc_lexer_get_token_e();

	return h;
}

static sbc_handle_t prv_priority2_e(void)
{
	return prv_priority1_e();
}

static sbc_handle_t prv_priority3_e(void)
{
	sbc_expression_t *e;
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_token_type_t tok_type;
	uint8_t op;

	e1 = prv_priority2_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	tok_type = lex.tok.type;

	while ((tok_type == SBC_TOKEN_OPERATOR) ||
	       (tok_type == SBC_TOKEN_KEYWORD)) {
		if (tok_type == SBC_TOKEN_OPERATOR) {
			op = sbc_exp_map_op();
			if ((op != SBC_EXP_MUL) && (op != SBC_EXP_RDIV))
				break;
		} else {
			op = lex.tok.tok.keyword;
			if (op == SBC_KEYWORD_DIV)
				op = SBC_EXP_DIV;
			else if (op == SBC_KEYWORD_MOD)
				op = SBC_EXP_MOD;
			else
				break;
		}

		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		e2 = prv_priority2_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (sbc_exp_start == sbc_exp_end) {
			err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
			return 0;
		}
		e = &sbc_expressions[sbc_exp_start];
		e->type = op;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;

		tok_type = lex.tok.type;
	}

	return e1;
}

static sbc_handle_t prv_priority4_e(void)
{
	sbc_expression_t *e;
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_token_type_t tok_type;
	uint8_t op;

	e1 = prv_priority3_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	tok_type = lex.tok.type;

	while ((tok_type == SBC_TOKEN_OPERATOR) ||
	       (tok_type == SBC_TOKEN_INTEGER) ||
	       (tok_type == SBC_TOKEN_REAL)) {
		if (tok_type == SBC_TOKEN_OPERATOR) {
			op = sbc_exp_map_op();
			if ((op != SBC_EXP_PLUS) && (op != SBC_EXP_MINUS))
				break;

			sbc_lexer_get_token_e();
			if (err_type != SPECASM_ERROR_OK)
				return 0;

		} else {
			/*
			 * Cope with unary minus
			 */

			if (((tok_type == SBC_TOKEN_INTEGER) &&
			    (lex.tok.tok.integer < 0)) ||
			    ((tok_type == SBC_TOKEN_REAL) &&
			     (lex.tok.tok.real.b[1] & 0x80)))
				op = SBC_EXP_PLUS;
			else
				break;
		}

		e2 = prv_priority3_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (sbc_exp_start == sbc_exp_end) {
			err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
			return 0;
		}
		e = &sbc_expressions[sbc_exp_start];
		e->type = op;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;
		tok_type = lex.tok.type;
	}

	return e1;
}

static sbc_handle_t prv_priority5_e(void)
{
	sbc_expression_t *e;
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_token_type_t tok_type;
	uint8_t op;

	e1 = prv_priority4_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	tok_type = lex.tok.type;

	while (tok_type == SBC_TOKEN_OPERATOR) {
		op = sbc_exp_map_op();
		switch (op) {
		case '=':
		case SBC_EXP_NEQ:
		case '<':
		case '>':
		case SBC_EXP_LTE:
		case SBC_EXP_GTE:
		case SBC_EXP_LSL:
		case SBC_EXP_LSR:
		case SBC_EXP_ASR:
			sbc_lexer_get_token_e();
			if (err_type != SPECASM_ERROR_OK)
				return 0;

			e2 = prv_priority4_e();
			if (err_type != SPECASM_ERROR_OK)
				return 0;

			if (sbc_exp_start == sbc_exp_end) {
				err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
				return 0;
			}
			e = &sbc_expressions[sbc_exp_start];
			e->type = op;
			e->v.args.a1 = e1;
			e->v.args.a2 = e2;
			e1 = sbc_exp_start++;
			tok_type = lex.tok.type;
			continue;
		}
		break;
	}

	return e1;
}

static sbc_handle_t prv_priority6_e(void)
{
	sbc_expression_t *e;
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_token_type_t tok_type;

	e1 = prv_priority5_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	tok_type = lex.tok.type;

	while ((tok_type == SBC_TOKEN_KEYWORD) &&
	       (lex.tok.tok.keyword == SBC_KEYWORD_AND)) {
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		e2 = prv_priority5_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (sbc_exp_start == sbc_exp_end) {
			err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
			return 0;
		}
		e = &sbc_expressions[sbc_exp_start];
		e->type = SBC_EXP_AND;;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;
		tok_type = lex.tok.type;
	}

	return e1;
}

static sbc_handle_t prv_priority7_e(void)
{
	sbc_expression_t *e;
	sbc_handle_t e1;
	sbc_handle_t e2;
	sbc_token_type_t tok_type;
	uint8_t op;

	e1 = prv_priority6_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	tok_type = lex.tok.type;

	while ((tok_type == SBC_TOKEN_KEYWORD) &&
	       ((lex.tok.tok.keyword == SBC_KEYWORD_OR) ||
		(lex.tok.tok.keyword == SBC_KEYWORD_EOR))) {
		op = (lex.tok.tok.keyword == SBC_KEYWORD_OR) ?
			SBC_EXP_OR : SBC_EXP_EOR;

		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		e2 = prv_priority6_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (sbc_exp_start == sbc_exp_end) {
			err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
			return 0;
		}
		e = &sbc_expressions[sbc_exp_start];
		e->type = op;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;
		tok_type = lex.tok.type;
	}

	return e1;
}

sbc_handle_t sbc_exp_parse_no_get_e(void)
{
	return prv_priority7_e();
}

sbc_handle_t sbc_exp_parse_e(void)
{
	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return prv_priority7_e();
}
