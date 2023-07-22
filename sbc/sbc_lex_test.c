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

#include "error.h"
#include "sbc_error.h"
#include "sbc_fmt_utils.h"
#include "sbc_lexer.h"


#define LEX_DUMP_MAX_INDENT 16

static uint8_t first_line = 1;
static uint8_t indent;
static uint8_t indent_stack[LEX_DUMP_MAX_INDENT];
static int8_t indent_stack_top = -1;
static uint8_t indent_done;

static void prv_dump_text(void)
{
	uint8_t i;

	for (i = 0; i < lex.tok.len; i++)
		putchar(lex.lex_buf[lex.tok.ptr + i]);
}

static uint8_t prv_adjust_keyword_indent(void)
{
	uint8_t *ptr;
	uint8_t old_indent = indent;

	switch (lex.tok.tok.keyword) {
	case SBC_KEYWORD_REPEAT:
	case SBC_KEYWORD_WHILE:
	case SBC_KEYWORD_FOR:
		if (indent_stack_top < LEX_DUMP_MAX_INDENT - 1) {
			indent_stack[++indent_stack_top] = indent++;
		}
		break;
	case SBC_KEYWORD_THEN:
		ptr = &lex.lex_buf[lex.tok.ptr + 1];
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		if (*ptr != 0xd)
			break;
		indent++;
		break;
	case SBC_KEYWORD_CASE:
		indent++;
		break;
	case SBC_KEYWORD_DEF:
		indent = 1;
		break;
	case SBC_KEYWORD_ENDWHILE:
	case SBC_KEYWORD_NEXT:
	case SBC_KEYWORD_UNTIL:
		if (indent_stack[indent_stack_top] != indent -1)
			break;
		indent_stack_top--;
	case SBC_KEYWORD_ENDIF:
	case SBC_KEYWORD_ENDPROC:
	case SBC_KEYWORD_ENDCASE:
		indent--;
		return indent;
	default:
		break;
	}

	return old_indent;
}

static void prv_do_indent(uint8_t ind)
{
	uint8_t i;

	if (!indent_done) {
		for (i = 0; i < ind * 4; i++)
			putchar(' ');
		indent_done = 1;
	}
}

static void prv_dump_e(const char *fname)
{
	uint8_t tokch;
	uint8_t i;


	sbc_lexer_open_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	do {
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			break;
		switch (lex.tok.type) {
		case SBC_TOKEN_LINE_LABEL:
			if (first_line)
				first_line = 0;
			else
				putchar('\n');
			printf("%5d ", lex.tok.tok.line_no);
			indent_done = 0;
			break;
		case SBC_TOKEN_IDENTIFIER:
			prv_do_indent(indent);
			prv_dump_text();
			break;
		case SBC_TOKEN_OPERATOR:
			tokch = lex.lex_buf[lex.tok.ptr];
			if (tokch != ',' && tokch != ':' && tokch != '(')
				putchar(' ');
			putchar(lex.lex_buf[lex.tok.ptr]);
			if (lex.tok.len > 1)
				putchar(lex.lex_buf[
						lex.tok.ptr + 1]);
			if (tokch != ':' && tokch != ')')
				putchar(' ');
			break;
		case SBC_TOKEN_KEYWORD:
			prv_do_indent(prv_adjust_keyword_indent());
			switch (lex.tok.tok.keyword) {
			case SBC_KEYWORD_OF:
			case SBC_KEYWORD_TO:
			case SBC_KEYWORD_STEP:
			case SBC_KEYWORD_THEN:
				putchar(' ');
				break;
			default:
				break;
			}
			printf("%s", sbc_fmt_keywords_strings[
				       lex.tok.tok.keyword]);
			switch (lex.tok.tok.keyword) {
			case SBC_KEYWORD_PROC:
			case SBC_KEYWORD_ERR:
				break;
			default:
				putchar(' ');
				break;
			}
			break;
		case SBC_TOKEN_INTEGER:
			printf("%d", lex.tok.tok.integer);
			break;
		case SBC_TOKEN_HEX:
			printf("&%X", lex.tok.tok.integer);
			break;
		case SBC_TOKEN_BIN:
			sbc_fmt_utils_dump_bin(&lex.tok.tok.integer);
			break;
		case SBC_TOKEN_REAL:
			sbc_fmt_utils_dump_real(&lex.tok.tok.real);
			break;
		case SBC_TOKEN_REM:
			printf("REM");
			for (i = 0; i < lex.tok.len; i++)
				putchar(lex.lex_buf[
						lex.tok.ptr + i]);
			break;
		case SBC_TOKEN_LINE_NUMBER:
			printf("%u", (unsigned int)
			       lex.tok.tok.line_no);
			break;
		case SBC_TOKEN_STRING:
			putchar('"');
			for (i = 0; i < lex.tok.len; i++)
				putchar(lex.lex_buf[
						lex.tok.ptr + i]);
			putchar('"');
		case SBC_TOKEN_EOF:
		case SBC_TOKEN_UNKNOWN:
			break;
		}
	} while (lex.tok.type != SBC_TOKEN_EOF);

	printf("\n");

	specasm_file_close_e(lex.h);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: sbcc .sbc\n");
		return 1;
	}

	prv_dump_e(argv[1]);

	if ((err_type == SBC_ERROR_OPEN) || (err_type == SBC_ERROR_WRITE)) {
		printf("%s : %s\n", sbc_error_msg(), argv[1]);
		return 1;
	}

	if (err_type != SPECASM_ERROR_OK) {
		printf("%s at line %d\n", sbc_error_msg(), lex.line_no);
		return 1;
	}

	return 0;
}

