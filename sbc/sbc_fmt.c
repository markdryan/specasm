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

#include <inttypes.h>
#include <string.h>

#include "sbc_exp.h"
#include "sbc_error.h"
#include "sbc_fmt_utils.h"
#include "sbc_parser.h"
#include "sbc_overlay.h"

static void prv_dump_block(uint16_t line_no, sbc_handle_t stmt, uint8_t ind);

static void prv_dump_text(sbc_big_handle_t ptr)
{
	uint8_t i;
	uint8_t* c = &sbc_pool_strings[ptr];
	uint8_t len = *c;

	for (i = 0; i < len; i++)
		putchar(*++c);
}

static void prv_dump_exp(sbc_handle_t exp)
{
	sbc_expression_t *e = &sbc_expressions[exp];

	/* We'll do the leaf nodes first */

	switch (e->type) {
	case SBC_EXP_STRING:
		putchar('"');
		prv_dump_text(e->v.id.str);
		putchar('"');
		return;
	case SBC_EXP_INTEGER:
		printf("%" PRIi32, e->v.integer);
		return;
	case SBC_EXP_HEX:
		printf("&%" PRIx32, e->v.integer);
		return;
	case SBC_EXP_BIN:
		sbc_fmt_utils_dump_bin(&e->v.integer);
		return;
	case SBC_EXP_REAL:
		sbc_fmt_utils_dump_real(&e->v.real);
		return;
        case SBC_EXP_IDENTIFIER:
		prv_dump_text(e->v.id.str);
		return;
	default:
		break;
	}

	/*
	 * Next we'll do the unary operators.
	 */

	switch (e->type) {
	case SBC_EXP_PEEKW:
	case SBC_EXP_PEEKB:
		putchar(e->type);
		prv_dump_exp(e->v.args.a1);
		return;
	case SBC_EXP_RND:
		printf("RND(");
		prv_dump_exp(e->v.args.a1);
		putchar(')');
		return;
	default:
		break;
	}

	/*
	 * That just leaves the binary operators.
	 */

	prv_dump_exp(e->v.args.a1);
	putchar(' ');
	switch (e->type) {
	case SBC_EXP_DIV:
		printf("DIV");
		break;
	case SBC_EXP_MOD:
		printf("MOD");
		break;
	case SBC_EXP_GTE:
		printf(">=");
		break;
	case SBC_EXP_LTE:
		printf("<=");
		break;
	case SBC_EXP_NEQ:
		printf("<>");
		break;
	case SBC_EXP_LSL:
		printf("<<");
		break;
	case SBC_EXP_LSR:
		printf(">>");
		break;
	case SBC_EXP_ASR:
		printf(">>>");
		break;
	case SBC_EXP_MUL:
	case SBC_EXP_PLUS:
	case SBC_EXP_MINUS:
	case SBC_EXP_RDIV:
	case SBC_EXP_LT:
	case SBC_EXP_EQ:
	case SBC_EXP_GT:
		putchar(e->type);
		break;
	default:
		break;
	}
	putchar(' ');

	prv_dump_exp(e->v.args.a2);
}

static void prv_dump_assign(sbc_handle_t stmt)
{
	uint8_t op;
	sbc_statement_t *s = &sbc_statements[stmt];

	prv_dump_text(s->d.assignment.id);
	putchar(' ');
	op = s->d.assignment.op;
	if (op == SBC_EXP_PLUSEQ)
		putchar('+');
	else if (op == SBC_EXP_MINUSEQ)
		putchar('-');
	putchar('=');
	putchar(' ');

	prv_dump_exp(s->d.assignment.exp);
}

static void prv_dump_four_exps(sbc_handle_t stmt, uint8_t count)
{
	sbc_statement_t *s;
	uint8_t i;

	s = &sbc_statements[stmt];
	prv_dump_exp(s->d.four_exps.e[0]);
	for (i = 1; i < count; i++) {
		putchar(',');
		putchar(' ');
		prv_dump_exp(s->d.four_exps.e[i]);
	}
}

static void prv_dump_for(sbc_handle_t stmt, uint8_t step)
{
	sbc_handle_t eh;
	sbc_statement_t *s = &sbc_statements[stmt];

	prv_dump_text(s->d.compound.id);
	printf(" = ");
	eh = s->d.compound.eh;

	prv_dump_exp(exp_list[eh].e);
	printf(" TO ");
	eh = exp_list[eh].next;
	prv_dump_exp(exp_list[eh].e);

	if (!step)
		return;

	eh = exp_list[eh].next;
	printf(" STEP ");
	prv_dump_exp(exp_list[eh].e);
}

static void prv_dump_node_list(sbc_handle_t node)
{
	if (node == SBC_MAX_EXP_NODES)
		return;
	prv_dump_exp(exp_list[node].e);
	node = exp_list[node].next;
	while (node != SBC_MAX_EXP_NODES) {
		putchar(',');
		putchar(' ' );
		prv_dump_exp(exp_list[node].e);
		node = exp_list[node].next;
	}
}

static void prv_dump_stmt(uint16_t line_no, sbc_handle_t stmt, uint8_t ind)
{
	sbc_statement_t *s;
	uint8_t i;
	uint8_t step = 0;

	s = &sbc_statements[stmt];
	if (line_no != s->line_no) {
		line_no = s->line_no;
		printf("\n%5d ", line_no);
		for (i = 0; i < ind * 4; i++)
			putchar(' ');
	} else if (stmt != 0) {
		putchar(':');
	}

	if (s->type == SBC_KEYWORD_ASSIGN) {
		prv_dump_assign(stmt);
		return;
	}

	if (s->type == SBC_KEYWORD_RECT_FILL)
		printf("RECTANGLE FILL ");
	else if (s->type == SBC_KEYWORD_FOR_STEP)
		printf("FOR");
	else if (s->type != SBC_KEYWORD_BLANK) {
		printf("%s", sbc_fmt_keywords_strings[s->type]);
		if (s->type != SBC_KEYWORD_REM)
			putchar(' ');
	}

	switch (s->type) {
	case SBC_KEYWORD_BLANK:
		putchar(':');
		break;
	case SBC_KEYWORD_CASE:
		prv_dump_text(s->d.compound.id);
		printf(" OF");
		prv_dump_block(line_no, s->d.compound.body, ind + 1);
		break;
	case SBC_KEYWORD_FOR_STEP:
		step = 1;
	case SBC_KEYWORD_FOR:
		prv_dump_for(stmt, step);
		prv_dump_block(line_no, s->d.compound.body, ind + 1);
		break;
	case SBC_KEYWORD_GCOL:
		prv_dump_exp(s->d.gcol.e1);
		if (s->d.gcol.count > 1) {
			putchar(',');
			putchar(' ');
			prv_dump_exp(s->d.gcol.e2);
		}
		break;
	case SBC_KEYWORD_GOTO:
	case SBC_KEYWORD_GOSUB:
		printf("%d", s->d.line_no);
		break;
	case SBC_KEYWORD_MODE:
		prv_dump_exp(s->d.exp);
		break;
	case SBC_KEYWORD_POINT2:
		prv_dump_four_exps(stmt, 2);
		break;
	case SBC_KEYWORD_PRINT:
		prv_dump_exp(s->d.exp);
		break;
	case SBC_KEYWORD_REM:
		prv_dump_text(s->d.str);
		break;
	case SBC_KEYWORD_RECT:
	case SBC_KEYWORD_RECT_FILL:
		prv_dump_four_exps(stmt, 4);
		break;
	case SBC_KEYWORD_WHEN:
		prv_dump_node_list(s->d.compound.eh);
		putchar(':');
		prv_dump_block(line_no, s->d.compound.body, ind + 1);
		break;
	case SBC_KEYWORD_WHILE:
		prv_dump_exp(s->d.compound.eh);
		prv_dump_block(line_no, s->d.compound.body, ind + 1);
		break;
	}
}

static void prv_dump_block(uint16_t line_no, sbc_handle_t stmt, uint8_t ind)
{
	do {
		prv_dump_stmt(line_no, stmt, ind);
		stmt = sbc_statements[stmt].next;
	} while (stmt != SBC_MAX_STATEMENTS);
}

static void prv_dump_e(const char *fname)
{
	uint16_t line_no;

	sbc_parse_file_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (sbc_statement_start == 0)
		return;
	line_no = sbc_statements[0].line_no;
	printf("%5d ", line_no);

	prv_dump_block(line_no, 0, 0);

	printf("\n");
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: sbcfmt .sbc\n");
		return 1;
	}

	prv_dump_e(argv[1]);

	if ((err_type == SBC_ERROR_OPEN) || (err_type == SBC_ERROR_WRITE)) {
		printf("%s : %s\n", sbc_error_msg(), argv[1]);
		return 1;
	}

	if (err_type != SPECASM_ERROR_OK) {
		printf("%s at line %d\n", sbc_error_msg(), overlay.lex.line_no);
		return 1;
	}

	return 0;
}

