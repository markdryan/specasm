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
#include "sbc_overlay.h"

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


sbc_handle_t sbc_exp_get_node_e(void)
{
	if (sbc_exp_list_start == (SBC_MAX_EXP_NODES) - 1) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return SBC_MAX_EXP_NODES - 1;
	}
	return sbc_exp_list_start++;
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
	uint8_t *str = &overlay.lex.lex_buf[t->ptr];

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

	if (overlay.lex.tok.len == 1)
		return overlay.lex.lex_buf[overlay.lex.tok.ptr];
	if (overlay.lex.tok.len == 3)
		return SBC_EXP_ASR;
	b1 = (char) overlay.lex.lex_buf[overlay.lex.tok.ptr];
	b2 = (char) overlay.lex.lex_buf[overlay.lex.tok.ptr + 1];

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

static sbc_handle_t prv_single_arg_fn(uint8_t op)
{
	sbc_expression_t *e;

	if (sbc_exp_start == sbc_exp_end) {
		err_type = SBC_ERROR_TOO_MANY_EXPRESSIONS;
		return 0;
	}

	e = &sbc_expressions[sbc_exp_start];
	e->type = op;
	e->v.args.a1 = sbc_exp_parse_e();

	return sbc_exp_start++;
}

static sbc_handle_t prv_priority1_e(void)
{
	sbc_token_type_t tok_type;
	sbc_handle_t e = 0;

	tok_type = overlay.lex.tok.type;
	switch (tok_type) {
	case SBC_TOKEN_INTEGER:
	case SBC_TOKEN_HEX:
	case SBC_TOKEN_BIN:
		e = sbc_exp_add_int_e(&overlay.lex.tok);
		break;
	case SBC_TOKEN_REAL:
		e = sbc_exp_add_real_e(&overlay.lex.tok);
		break;
	case SBC_TOKEN_IDENTIFIER:
		e = sbc_exp_add_id_e(&overlay.lex.tok);
		break;
	case SBC_TOKEN_STRING:
		e = sbc_exp_add_string_e(&overlay.lex.tok);
		break;
	case SBC_TOKEN_OPERATOR:
		switch (sbc_exp_map_op()) {
		case SBC_EXP_OPENB:
			e = sbc_exp_parse_e();
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			if ((overlay.lex.tok.type != SBC_TOKEN_OPERATOR) ||
			    (sbc_exp_map_op() != SBC_EXP_CLOSEB)) {
				err_type = SBC_ERROR_CLOSEB_EXPECTED;
				return 0;
			}
		}
		break;
	case SBC_TOKEN_KEYWORD:
		switch (overlay.lex.tok.tok.keyword) {
		case SBC_KEYWORD_RND:
			return prv_single_arg_fn(SBC_EXP_RND);
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

	return e;
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

	tok_type = overlay.lex.tok.type;

	while ((tok_type == SBC_TOKEN_OPERATOR) ||
	       (tok_type == SBC_TOKEN_KEYWORD)) {
		if (tok_type == SBC_TOKEN_OPERATOR) {
			op = sbc_exp_map_op();
			if ((op != SBC_EXP_MUL) && (op != SBC_EXP_RDIV))
				break;
		} else {
			op = overlay.lex.tok.tok.keyword;
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

		e = &sbc_expressions[sbc_exp_start];
		e->type = op;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;

		tok_type = overlay.lex.tok.type;
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

	tok_type = overlay.lex.tok.type;

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
			    (overlay.lex.tok.tok.integer < 0)) ||
			    ((tok_type == SBC_TOKEN_REAL) &&
			     (overlay.lex.tok.tok.real.b[1] & 0x80)))
				op = SBC_EXP_PLUS;
			else
				break;
		}

		e2 = prv_priority3_e();
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		e = &sbc_expressions[sbc_exp_start];
		e->type = op;
		e->v.args.a1 = e1;
		e->v.args.a2 = e2;
		e1 = sbc_exp_start++;
		tok_type = overlay.lex.tok.type;
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

	tok_type = overlay.lex.tok.type;

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

			e = &sbc_expressions[sbc_exp_start];
			e->type = op;
			e->v.args.a1 = e1;
			e->v.args.a2 = e2;
			e1 = sbc_exp_start++;
			tok_type = overlay.lex.tok.type;
			continue;
		}
		break;
	}

	return e1;
}

sbc_handle_t sbc_exp_parse_no_get_e(void)
{
	return prv_priority5_e();
}

sbc_handle_t sbc_exp_parse_e(void)
{
	sbc_lexer_get_token_e();
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return prv_priority5_e();
}
