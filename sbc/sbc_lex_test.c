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
#include "sbc_lexer.h"
#include "sbc_overlay.h"

#define LEX_DUMP_MAX_INDENT 16

static uint8_t first_line = 1;
static uint8_t indent;
static uint8_t indent_stack[LEX_DUMP_MAX_INDENT];
static int8_t indent_stack_top = -1;
static uint8_t indent_done;

static const char * const keywords_strings[] = {
	"OTHERWISE", // SBC_KEYWORD_OTHERWISE
	"AND",       // SBC_KEYWORD_AND
	"DIV",       // SBC_KEYWORD_DIV
	"EOR", // SBC_KEYWORD_EOR
	"MOD", // SBC_KEYWORD_MOD
	"OR", // SBC_KEYWORD_OR
	"ERROR", // SBC_KEYWORD_ERROR
	"LINE", // SBC_KEYWORD_LINE
	"OFF", // SBC_KEYWORD_OFF
	"STEP", // SBC_KEYWORD_STEP
	"SPC", // SBC_KEYWORD_SPC
	"TAB(", // SBC_KEYWORD_TAB
	"ELSE", // SBC_KEYWORD_ELSE
	"THEN", // SBC_KEYWORD_THEN
	"",
	"OPENIN", // SBC_KEYWORD_OPENIN
	"PTR", // SBC_KEYWORD_PTR
	"PAGE", // SBC_KEYWORD_PAGE
	"TIME", // SBC_KEYWORD_TIME
	"LOMEM", // SBC_KEYWORD_LOMEM
	"HIMEM", // SBC_KEYWORD_HIMEM
	"ABS", // SBC_KEYWORD_ABS
	"ACS", // SBC_KEYWORD_ACS
	"ADVAL", // SBC_KEYWORD_ADVAL
	"ASC", // SBC_KEYWORD_ASC
	"ASN", // SBC_KEYWORD_ASN
	"ATN", // SBC_KEYWORD_ATN
	"BGET", // SBC_KEYWORD_BGET
	"COS", // SBC_KEYWORD_COS
	"COUNT", // SBC_KEYWORD_COUNT
	"DEG", // SBC_KEYWORD_DEG
	"ERL", // SBC_KEYWORD_ERL
	"ERR", // SBC_KEYWORD_ERR
	"EVAL", // SBC_KEYWORD_EVAL
	"EXP", // SBC_KEYWORD_EXP
	"EXT", // SBC_KEYWORD_EXT
	"FALSE", // SBC_KEYWORD_FALSE
	"FN", // SBC_KEYWORD_FN
	"GET", // SBC_KEYWORD_GET
	"INKEY", // SBC_KEYWORD_INKEY
	"INSTR(", // SBC_KEYWORD_INSTR
	"INT", // SBC_KEYWORD_INT
	"LEN", // SBC_KEYWORD_LEN
	"LN", // SBC_KEYWORD_LN
	"LOG", // SBC_KEYWORD_LOG
	"NOT", // SBC_KEYWORD_NOT
	"OPENUP", // SBC_KEYWORD_OPENUP
	"OPENOUT", // SBC_KEYWORD_OPENOUT
	"PI", // SBC_KEYWORD_PI
	"POINT(", // SBC_KEYWORD_POINT
	"POS", // SBC_KEYWORD_POS
	"RAD", // SBC_KEYWORD_RAD
	"RND", // SBC_KEYWORD_RND2
	"SGN", // SBC_KEYWORD_SGN
	"SIN", // SBC_KEYWORD_SIN
	"SQR", // SBC_KEYWORD_SQR
	"TAN", // SBC_KEYWORD_TAN
	"TO", // SBC_KEYWORD_TO
	"TRUE", // SBC_KEYWORD_TRUE
	"USR", // SBC_KEYWORD_USR
	"VAL", // SBC_KEYWORD_VAL
	"VPOS", // SBC_KEYWORD_VPOS
	"CHR$", // SBC_KEYWORD_CHR_STR
	"GET$", // SBC_KEYWORD_GET_STR
	"INKEY$", // SBC_KEYWORD_INKEY_STR
	"LEFT$(", // SBC_KEYWORD_LEFT_STR
	"MID$(", // SBC_KEYWORD_MID_STR
	"RIGHT$(", // SBC_KEYWORD_RIGHT_STR
	"STR$", // SBC_KEYWORD_STR_STR
	"STRING$(", // SBC_KEYWORD_STRING_STR
	"EOF", // SBC_KEYWORD_EOF_HASH
	"",
	"",
	"",
	"WHEN", // SBC_KEYWORD_WHEN
	"OF", // SBC_KEYWORD_OF
	"ENDCASE", // SBC_KEYWORD_ENDCASE
	"ELSE", // SBC_KEYWORD_ELSE_2
	"ENDIF", // SBC_KEYWORD_ENDIF
	"ENDWHILE", // SBC_KEYWORD_ENDWHILE
	"PTR", // SBC_KEYWORD_PTR_2
	"PAGE", // SBC_KEYWORD_PAGE_2
	"TIME", // SBC_KEYWORD_TIME_2
	"LOMEM", // SBC_KEYWORD_LOMEM_2
	"HIMEM", // SBC_KEYWORD_HIMEM_2
	"SOUND", // SBC_KEYWORD_SOUND
	"BPUT", // SBC_KEYWORD_BPUT
	"CALL", // SBC_KEYWORD_CALL
	"CHAIN", // SBC_KEYWORD_CHAIN
	"CLEAR", // SBC_KEYWORD_CLEAR
	"CLOSE", // SBC_KEYWORD_CLOSE
	"CLG", // SBC_KEYWORD_CLG
	"CLS", // SBC_KEYWORD_CLS
	"DATA", // SBC_KEYWORD_DATA
	"DEF", // SBC_KEYWORD_DEF
	"DIM", // SBC_KEYWORD_DIM
	"DRAW", // SBC_KEYWORD_DRAW
	"END", // SBC_KEYWORD_END
	"ENDPROC", // SBC_KEYWORD_ENDPROC
	"ENVELOPE", // SBC_KEYWORD_ENVELOPE
	"FOR", // SBC_KEYWORD_FOR
	"GOSUB", // SBC_KEYWORD_GOSUB
	"GOTO", // SBC_KEYWORD_GOTO
	"GCOL", // SBC_KEYWORD_GCOL
	"IF", // SBC_KEYWORD_IF
	"INPUT", // SBC_KEYWORD_INPUT
	"LET", // SBC_KEYWORD_LET
	"LOCAL", // SBC_KEYWORD_LOCAL
	"MODE", // SBC_KEYWORD_MODE
	"MOVE", // SBC_KEYWORD_MOVE
	"NEXT", // SBC_KEYWORD_NEXT
	"ON", // SBC_KEYWORD_ON
	"VDU", // SBC_KEYWORD_VDU
	"PLOT", // SBC_KEYWORD_PLOT
	"PRINT", // SBC_KEYWORD_PRINT
	"PROC", // SBC_KEYWORD_PROC
	"READ", // SBC_KEYWORD_READ
	"REM", // SBC_KEYWORD_REM
	"REPEAT", // SBC_KEYWORD_REPEAT
	"REPORT", // SBC_KEYWORD_REPORT
	"RESTORE", // SBC_KEYWORD_RESTORE
	"RETURN", // SBC_KEYWORD_RETURN
	"RUN", // SBC_KEYWORD_RUN
	"STOP", // SBC_KEYWORD_STOP
	"COLOUR", // SBC_KEYWORD_COLOUR
	"TRACE", // SBC_KEYWORD_TRACE
	"UNTIL", // SBC_KEYWORD_UNTIL
	"WIDTH", // SBC_KEYWORD_WIDTH
	"OSCLI", // SBC_KEYWORD_OSCLI

	"CASE", // SBC_KEYWORD_CASE
	"CIRCLE", // SBC_KEYWORD_CIRCLE
	"FILL", // SBC_KEYWORD_FILL
	"ORIGIN", // SBC_KEYWORD_ORIGIN
	"POINT", // SBC_KEYWORD_POINT2
	"RECTANGLE", // SBC_KEYWORD_RECT
	"SWAP", // SBC_KEYWORD_SWAP
	"WHILE", // SBC_KEYWORD_WHILE
	"WAIT", // SBC_KEYWORD_WAIT
	"MOUSE", // SBC_KEYWORD_MOUSE
	"QUIT", // SBC_KEYWORD_QUIT
	"SYS", // SBC_KEYWORD_SYS
	"INSTALL", // SBC_KEYWORD_INSTALL
	"LIBRARY", // SBC_KEYWORD_LIBRARY
	"TINT", // SBC_KEYWORD_TINT
	"ELLIPSE", // SBC_KEYWORD_ELLIPSE
	"BEATS", // SBC_KEYWORD_BEATS
	"TEMPO", // SBC_KEYWORD_TEMPO
	"VOICES", // SBC_KEYWORD_VOICES
	"VOICE", // SBC_KEYWORD_VOICE
	"STEREO", // SBC_KEYWORD_STEREO
	"OVERLAY", // SBC_KEYWORD_OVERLAY

	"APPEND", // SBC_KEYWORD_APPEND
	"AUTO", // SBC_KEYWORD_AUTO
	"CRUNCH", // SBC_KEYWORD_CRUNCH
	"DELETE", // SBC_KEYWORD_DELETE
	"EDIT", // SBC_KEYWORD_EDIT
	"HELP", // SBC_KEYWORD_HELP
	"LIST", // SBC_KEYWORD_LIST
	"LOAD", // SBC_KEYWORD_LOAD
	"LVAR", // SBC_KEYWORD_LVAR
	"NEW", // SBC_KEYWORD_NEW
	"OLD", // SBC_KEYWORD_OLD
	"RENUMBER", // SBC_KEYWORD_RENUMBER
	"SAVE", // SBC_KEYWORD_SAVE
	"TEXTLOAD", // SBC_KEYWORD_TEXTLOAD
	"TEXTSAVE", // SBC_KEYWORD_TEXTSAVE
	"TWIN", // SBC_KEYWORD_TWIN
	"TWINNO", // SBC_KEYWORD_TWINNO
	"INSTALL2", // SBC_KEYWORD_INSTALL2

	"SUM", // SBC_KEYWORD_SUM
	"BEAT", // SBC_KEYWORD_BEAT
};

static void prv_dump_text(void)
{
	uint8_t i;

	for (i = 0; i < overlay.lex.tok.len; i++)
		putchar(overlay.lex.lex_buf[overlay.lex.tok.ptr + i]);
}

static uint8_t prv_adjust_keyword_indent(void)
{
	uint8_t *ptr;
	uint8_t old_indent = indent;

	switch (overlay.lex.tok.tok.keyword) {
	case SBC_KEYWORD_REPEAT:
	case SBC_KEYWORD_WHILE:
	case SBC_KEYWORD_FOR:
		if (indent_stack_top < LEX_DUMP_MAX_INDENT - 1) {
			indent_stack[++indent_stack_top] = indent++;
		}
		break;
	case SBC_KEYWORD_THEN:
		ptr = &overlay.lex.lex_buf[overlay.lex.tok.ptr + 1];
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

static void prv_dump_real(void)
{
	float real;
	uint8_t bin_num[4];
	uint8_t exponent = overlay.lex.tok.tok.real.b[0] - 1;

	bin_num[0] = overlay.lex.tok.tok.real.b[3];
	bin_num[1] = overlay.lex.tok.tok.real.b[2];
	bin_num[3] = overlay.lex.tok.tok.real.b[1] & 0x80;
	bin_num[2] = overlay.lex.tok.tok.real.b[1] & 0x7f;
	bin_num[3] |=  exponent >> 1;
	if (exponent & 1)
		bin_num[2] |= 0x80;

	memcpy(&real, bin_num, sizeof(float));

	printf("%f", real);
}

static void prv_dump_e(const char *fname)
{
	uint8_t tokch;
	uint8_t i;
	uint32_t mask;

	sbc_lexer_open_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	do {
		sbc_lexer_get_token_e();
		if (err_type != SPECASM_ERROR_OK)
			break;
		switch (overlay.lex.tok.type) {
		case SBC_TOKEN_LINE_LABEL:
			if (first_line)
				first_line = 0;
			else
				putchar('\n');
			printf("%5d ", overlay.lex.tok.tok.line_no);
			indent_done = 0;
			break;
		case SBC_TOKEN_IDENTIFIER:
			prv_do_indent(indent);
			prv_dump_text();
			break;
		case SBC_TOKEN_OPERATOR:
			tokch = overlay.lex.lex_buf[overlay.lex.tok.ptr];
			if (tokch != ',' && tokch != ':' && tokch != '(')
				putchar(' ');
			putchar(overlay.lex.lex_buf[overlay.lex.tok.ptr]);
			if (overlay.lex.tok.len > 1)
				putchar(overlay.lex.lex_buf[
						overlay.lex.tok.ptr + 1]);
			if (tokch != ':' && tokch != ')')
				putchar(' ');
			break;
		case SBC_TOKEN_KEYWORD:
			prv_do_indent(prv_adjust_keyword_indent());
			switch (overlay.lex.tok.tok.keyword) {
			case SBC_KEYWORD_OF:
			case SBC_KEYWORD_TO:
			case SBC_KEYWORD_STEP:
			case SBC_KEYWORD_THEN:
				putchar(' ');
				break;
			default:
				break;
			}
			printf("%s", keywords_strings[
				       overlay.lex.tok.tok.keyword]);
			switch (overlay.lex.tok.tok.keyword) {
			case SBC_KEYWORD_PROC:
			case SBC_KEYWORD_ERR:
				break;
			default:
				putchar(' ');
				break;
			}
			break;
		case SBC_TOKEN_INTEGER:
			printf("%d", overlay.lex.tok.tok.integer);
			break;
		case SBC_TOKEN_HEX:
			printf("&%X", overlay.lex.tok.tok.integer);
			break;
		case SBC_TOKEN_BIN:
			mask = 0x80000000;
			putchar('%');
			while (mask && !(mask & overlay.lex.tok.tok.integer))
				mask >>= 1;
			if (mask == 0) {
				putchar('0');
				break;
			}
			while (mask) {
				i = (mask & overlay.lex.tok.tok.integer) ? 1 :
					0;
				putchar(i + '0');
				mask >>= 1;
			}
			break;
		case SBC_TOKEN_REAL:
			prv_dump_real();
			break;
		case SBC_TOKEN_REM:
			printf("REM");
			for (i = 1; i < overlay.lex.tok.len; i++)
				putchar(overlay.lex.lex_buf[
						overlay.lex.tok.ptr + i]);
			break;
		case SBC_TOKEN_LINE_NUMBER:
			printf("%u", (unsigned int)
			       overlay.lex.tok.tok.line_no);
			break;
		case SBC_TOKEN_STRING:
			putchar('"');
			for (i = 0; i < overlay.lex.tok.len; i++)
				putchar(overlay.lex.lex_buf[
						overlay.lex.tok.ptr + i]);
			putchar('"');
		case SBC_TOKEN_EOF:
		case SBC_TOKEN_UNKNOWN:
			break;
		}
	} while (overlay.lex.tok.type != SBC_TOKEN_EOF);

	printf("\n");

	specasm_file_close_e(overlay.lex.h);
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
		printf("%s at line %d\n", sbc_error_msg(), overlay.lex.line_no);
		return 1;
	}

	return 0;
}

