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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "expression.h"
#include "salink.h"
#include "state_base.h"

/*
 * This is all a bit complicated so is worth documenting for future
 * reference.
 *
 * equ statements in .x files are treated as special kinds of labels.
 * However, they contain not one but two strings.  The type and id of the
 * first string (the name of the constant) are stored in op_code[0]
 * and op_code[1].  The type and id of the second are stored in
 * op_code[2] and op_code[3].
 *
 * When we process the object files in the linker for the first time
 * we add the equ statements just as we add normal labels.  We store
 * the type and id of the second string in the data.equ array.
 *
 * Special care is needed for global variables.
 *
 * 1. We store the expression itself, in the global variables name
 *    as a second null terminated string.  This is needed as we
 *    need access to the variable name and expression when processing
 *    files that might not be loaded.
 * 2. Once we've processed all the .x files we then evaluate all the
 *    global expressions.  This is possible as the strings for all the
 *    global expressions are now in memory.  Global labels cannot
 *    refer to local labels, as those labels may not be loaded into
 *    memory.  Once the expression has been evaluated the type of the
 *    label is altered to express that fact and the result of the
 *    expression is stored in the label offset.
 *
 * The expressions used in local equ statements and expressions used
 * directly in instructions are then evaluated when we write the
 * instruction's data out to the final binary, after normal labels
 * have been resolved.  Instructions that contain expressions
 * have a different type, their normal type + SPECASM_LINE_TYPE_EXP_ADJ.
 * The type and id of the string containing the expressions is stored
 * in the line's address format and in a byte somewhere in the opcode
 * array, either byte 1 or 2.  To process the expression we use the
 * type and id to locate the string containing the expression which we
 * then pass to the expression parser.  If the expression contains
 * a label, either a real label or an equ statement, we need to use
 * the string of the label to do a reverse label look up.  This is
 * different from normal label lookup which is done using the string
 * id, rather than the string itself.  Once encounter a local
 * unevaluated EQU statement reference, we evaluate it and change
 * its type and cache the value so it doesn't need re-evaluated if
 * it is encountered again in the future.  8 levels of nesting are
 * permitted.
 *
 * We need long and short versions of the local equ label types so
 * we can look up their strings when performing the string match.
 *
 */

#define SALINK_TOKEN_EOF 0
#define SALINK_TOKEN_NUM 1
#define SALINK_TOKEN_LOCAL_LABEL 2
#define SALINK_TOKEN_GLOBAL_LABEL 3
#define SALINK_TOKEN_OP 4

#define SALINK_TOKEN_LSL_VAL 1
#define SALINK_TOKEN_ASR_VAL 2

#define SALINK_MAX_DEPTH 8

struct salink_token_t_ {
	uint8_t type;
	union {
		int16_t num;
		uint8_t id;
	} data;
};

typedef struct salink_token_t_ salink_token_t;

struct salink_exp_stack_entry_t_ {
	uint8_t is_global;
	uint8_t depth;
	uint16_t line_no;
};

typedef struct salink_exp_stack_entry_t_ salink_exp_stack_entry_t;

static salink_exp_stack_entry_t g_stack[SALINK_MAX_DEPTH + 1];
static uint8_t g_stack_top;

static salink_obj_t *g_obj;

static const char *prv_exp_priority4_e(const char *str, int16_t *e);

static void prv_equ_eval_local_e(salink_label_t *label, uint8_t depth,
				 uint16_t line_no);
void salink_equ_eval_global_banked_e(salink_obj_t *obj, salink_global_t *global,
				     salink_label_t *label, uint8_t depth);
static const char simple_ops[] = "()/*+-&|^~";

static const char zx81_conseq_ops[] = {'"', '#', '$', ':', '?', '(',
				       ')', '>', '<', '=', '+', '-',
				       '*', '/', ';', ',', '.'};

#ifdef SPECASM_NEXT_BANKED
char salink_to_zx81_char_banked(char ch)
#else
char salink_to_zx81_char(char ch)
#endif
{
	uint8_t i;

	if (ch == ' ')
		return 0;

	for (i = 0; i < sizeof(zx81_conseq_ops); i++)
		if (ch == zx81_conseq_ops[i])
			return 11 + i;

	if ((ch >= '0') && (ch <= '9'))
		return ch - 20;

	ch |= 32;
	if ((ch >= 'a') && (ch <= 'z'))
		return ch - 59;

	return 15; // '?'
}

static void prv_unknown_error_label_e(salink_obj_t *obj, const char *str)
{
	snprintf(error_buf, sizeof(error_buf), "%s:Unknown label in equ:%s",
		 obj->fname, str);
	err_type = SALINK_ERROR_UNRESOLVED_LABEL;
}

/*
 * Find a label id given a string.  We need this when evaluating expressions.
 * Labels within expressions are encoded as strings rather than as ids.
 */

static unsigned int prv_find_local_label_e(const char *str, int len,
					   salink_obj_t *obj)
{
	unsigned int i;
	salink_label_t *label;
	const char *lab_str;
	uint8_t lng = len >= SPECASM_MAX_SHORT_LEN ? 1 : 0;
	uint8_t lab_lng;

	for (i = obj->label_start; i < obj->label_end; i++) {
		label = &labels[i];
		if ((label->type == SALINK_LABEL_TYPE_ALIGN) ||
		    (label->type == SALINK_LABEL_TYPE_EQU_GLOBAL) ||
		    (label->type == SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL))
			continue;
		lab_lng = label->type & 1;
		if (lng != lab_lng)
			continue;

		lab_str = salink_get_label_str_e(label->id, lng);
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (!strcmp(str, lab_str))
			return i;
	}

	prv_unknown_error_label_e(obj, str);

	return 0;
}

/*
 * Returns the index of the global that matches str
 */

static unsigned int prv_find_global_label_e(const char *str, salink_obj_t *obj)
{
	unsigned int i;
	salink_global_t *global;

	for (i = 0; i < global_count; i++) {
		global = &globals[i];
		if (!strcmp(str, global->name))
			return i;
	}

	prv_unknown_error_label_e(obj, str);

	return 0;
}

const char *prv_get_token_e(const char *buf, salink_token_t *tok,
			    uint8_t is_global)
{
	const char *start;
	char *end_ptr;
	uint8_t i;
	char c;
	char ch;
	long lval;
	int len;

	if (!buf) {
		tok->type = SALINK_TOKEN_EOF;
		return NULL;
	}

	while (*buf == ' ')
		buf++;
	c = *buf;

	if (c == 0) {
		tok->type = SALINK_TOKEN_EOF;
		return NULL;
	}

	if (c == '\'') {
		buf++;
		ch = *buf;
		if (!ch || ch == '\'' || buf[1] != '\'') {
			err_type = SALINK_ERROR_BAD_EXPRESSION;
			return NULL;
		}
		tok->type = SALINK_TOKEN_NUM;
		if (got_zx81)
#ifdef SPECASM_NEXT_BANKED
			ch = salink_to_zx81_char_banked(ch);
#else
			ch = salink_to_zx81_char(ch);
#endif
		tok->data.num = (int16_t)ch;
		return buf + 2;
	}

	if ((c == '$') || ((c >= '0') && (c <= '9'))) {
		if (c == '$')
			buf++;
		lval = strtol((char *)buf, &end_ptr, c == '$' ? 16 : 10);

		if ((lval > 0xffff) || (lval < -32768)) {
			err_type = SPECASM_ERROR_NUM_TOO_BIG;
			return NULL;
		}
		tok->data.num = (int16_t)lval;
		if (end_ptr == (char *)buf) {
			err_type = SPECASM_ERROR_BAD_NUM;
			return 0;
		}
		tok->type = SALINK_TOKEN_NUM;
		return end_ptr;
	}

	for (i = 0; i < sizeof(simple_ops) - 1; i++)
		if (c == simple_ops[i]) {
			tok->type = SALINK_TOKEN_OP;
			tok->data.id = c;
			return buf + 1;
		}

	if ((c == '<') && (buf[1] == '<')) {
		tok->type = SALINK_TOKEN_OP;
		tok->data.id = SALINK_TOKEN_LSL_VAL;
		return buf + 2;
	}

	if ((c == '>') && (buf[1] == '>')) {
		tok->type = SALINK_TOKEN_OP;
		tok->data.id = SALINK_TOKEN_ASR_VAL;
		return buf + 2;
	}

	if ((c == '_') || ((c >= 'a') && (c <= 'z')) ||
	    ((c >= 'A') && (c <= 'Z'))) {
		start = buf;
		buf++;
		c = *buf;
		while (((c >= 'a') && (c <= 'z')) ||
		       ((c >= 'A') && (c <= 'Z')) ||
		       ((c >= '0') && (c <= '9')) || (c == '_')) {
			buf++;
			c = *buf;
		}
		len = buf - start;
		memcpy(scratch, start, len);
		scratch[len] = 0;

		c = *start;
		if ((c >= 'A') && (c <= 'Z')) {
			tok->data.id =
			    prv_find_global_label_e(scratch, g_obj);
			tok->type = SALINK_TOKEN_GLOBAL_LABEL;
		} else {
			tok->data.id = prv_find_local_label_e(scratch, len,
							      g_obj);
			if ((err_type == SPECASM_ERROR_ASSERT_BAD_STRING_ID) &&
			    (is_global)) {
				err_type = SALINK_ERROR_LOCAL_IN_GLOBAL_EQU;
				return NULL;
			}
			tok->type = SALINK_TOKEN_LOCAL_LABEL;
		}

		return buf;
	}

	err_type = SALINK_ERROR_BAD_EXPRESSION;
	return NULL;
}

static const char *prv_exp_priority0_e(const char *str, int16_t *e)
{
	salink_token_t tok;
	salink_label_t *label;
	salink_global_t *global;
	uint8_t is_global = g_stack[g_stack_top -1].is_global;
	uint16_t line_no = g_stack[g_stack_top - 1].line_no;
	uint8_t depth = g_stack[g_stack_top - 1].depth;

	str = prv_get_token_e(str, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK) {
		return NULL;
	}

	switch (tok.type) {
	case SALINK_TOKEN_OP:
		str = prv_exp_priority4_e(str, e);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
		switch (tok.data.id) {
		case '-':
			*e = -*e;
			break;
		case '~':
			*e = ~*e;
			break;
		case '(':
			str = prv_get_token_e(str, &tok, is_global);
			if (err_type != SPECASM_ERROR_OK)
				return NULL;
			if ((tok.type == SALINK_TOKEN_OP) &&
			    (tok.data.id == ')'))
				break;
		default:
			err_type = SALINK_ERROR_BAD_EXPRESSION;
			return NULL;
		}
		break;
	case SALINK_TOKEN_EOF:
		err_type = SALINK_ERROR_BAD_EXPRESSION;
		return NULL;
	case SALINK_TOKEN_NUM:
		*e = tok.data.num;
		break;
	case SALINK_TOKEN_LOCAL_LABEL:
		if (is_global) {
			err_type = SALINK_ERROR_LOCAL_IN_GLOBAL_EQU;
			return NULL;
		}
		label = &labels[tok.data.id];
		switch (label->type) {
		case SALINK_LABEL_TYPE_SHORT:
		case SALINK_LABEL_TYPE_LNG:
		case SALINK_LABEL_TYPE_EQU_EVAL_SHORT:
		case SALINK_LABEL_TYPE_EQU_EVAL_LONG:
			*e = label->data.off;
			break;
		case SALINK_LABEL_TYPE_EQU_SHORT:
		case SALINK_LABEL_TYPE_EQU_LONG:
			if (depth == SALINK_MAX_DEPTH) {
				err_type = SALINK_ERROR_RECURISVE_EQU;
				return NULL;
			}
			prv_equ_eval_local_e(label, depth + 1, line_no);
			if (err_type != SPECASM_ERROR_OK)
				return NULL;
			*e = label->data.off;
			break;
		default:
			/* Should not happen so not worth a proper error */

			err_type = SPECASM_ERROR_BAD_LABEL;
			return NULL;
		}
		break;
	case SALINK_TOKEN_GLOBAL_LABEL:
		global = &globals[tok.data.id];
		label = &labels[global->label_index];
		switch (label->type) {
		case SALINK_LABEL_TYPE_SHORT:
		case SALINK_LABEL_TYPE_LNG:
		case SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL:
			*e = label->data.off;
			break;
		case SALINK_LABEL_TYPE_EQU_GLOBAL:
			if (depth == SALINK_MAX_DEPTH) {
				err_type = SALINK_ERROR_RECURISVE_EQU;
				return NULL;
			}
#ifdef SPECASM_NEXT_BANKED
			salink_equ_eval_global_banked_e(g_obj, global, label,
							depth + 1);
#else
			salink_equ_eval_global_e(g_obj, global, label,
						 depth + 1);
#endif
			if (err_type != SPECASM_ERROR_OK)
				return NULL;
			*e = label->data.off;
			break;
		default:
			/* Should not happen so not worth a proper error */

			err_type = SPECASM_ERROR_BAD_LABEL;
			return NULL;
		}
		break;
	default:
		err_type = SPECASM_ERROR_BAD_LABEL;
		return NULL;
	}

	return str;
}

static const char *prv_exp_priority1_e(const char *str, int16_t *e)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;
	uint8_t is_global = g_stack[g_stack_top - 1].is_global;

	str = prv_exp_priority0_e(str, &e1);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '*' && op != '/' && op != '%')
			break;

		str = prv_exp_priority0_e(next, &e2);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;

		if (op != '*' && e2 == 0) {
			err_type = SALINK_ERROR_DIV_ZERO;
			return NULL;
		}

		switch (op) {
		case '*':
			e1 = e1 * e2;
			break;
		case '/':
			e1 = e1 / e2;
			break;
		case '%':
			e1 = e1 % e2;
			break;
		}
		next = prv_get_token_e(str, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority2_e(const char *str, int16_t *e)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;
	uint8_t is_global = g_stack[g_stack_top - 1].is_global;

	str = prv_exp_priority1_e(str, &e1);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '+' && op != '-')
			break;

		str = prv_exp_priority1_e(next, &e2);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;

		switch (op) {
		case '+':
			e1 = e1 + e2;
			break;
		case '-':
			e1 = e1 - e2;
			break;
		}
		next = prv_get_token_e(str, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority3_e(const char *str, int16_t *e)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;
	uint8_t is_global = g_stack[g_stack_top - 1].is_global;

	str = prv_exp_priority2_e(str, &e1);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if ((op != SALINK_TOKEN_LSL_VAL) &&
		    (op != SALINK_TOKEN_ASR_VAL))
			break;

		str = prv_exp_priority2_e(next, &e2);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;

		switch (op) {
		case SALINK_TOKEN_LSL_VAL:
			e1 = e1 << e2;
			break;
		case SALINK_TOKEN_ASR_VAL:
			e1 = e1 >> e2;
			break;
		}
		next = prv_get_token_e(str, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority4_e(const char *str, int16_t *e)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;
	uint8_t is_global = g_stack[g_stack_top - 1].is_global;

	str = prv_exp_priority3_e(str, &e1);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '&' && op != '|' && op != '^')
			break;

		str = prv_exp_priority3_e(next, &e2);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;

		switch (op) {
		case '&':
			e1 = e1 & e2;
			break;
		case '|':
			e1 = e1 | e2;
			break;
		case '^':
			e1 = e1 ^ e2;
			break;
		}
		next = prv_get_token_e(str, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static int16_t prv_equ_eval_e(const char *name, uint8_t depth,
			      uint8_t is_global, uint16_t line_no)
{
	const char *str;
	int16_t e;
	salink_token_t tok;

	g_stack[g_stack_top].is_global = is_global;
	g_stack[g_stack_top].depth = depth;
	g_stack[g_stack_top].line_no = line_no;
	g_stack_top++;

	str = prv_exp_priority4_e(name, &e);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	g_stack_top--;

	(void)prv_get_token_e(str, &tok, is_global);
	if (tok.type != SALINK_TOKEN_EOF)
		err_type = SALINK_ERROR_BAD_EXPRESSION;

	return e;
}

/*
 * There are three places expressions can occur
 * 1. In the definition of Global labels
 * 2. In an instruction or data directive
 * 3. In the definition of a local label
 *
 * If there's an error with the expression we want to
 * print out the line number at fault.  We always have this
 * informtion for cases 1 and 2 but we don't always have it
 * for case 3, in the case where the error is in a separate
 * local label, referenced by the label we're currently
 * processing.  In this case we can print out the contents
 * of the invalid equ statement but we can't provide its line
 * number.  We can only provide the line number of the original
 * local equ statement that caused the faulty equ statement to be
 * evaluated.
 */

static void prv_check_exp_err(const char *name, uint16_t line_no,
			      uint8_t exact_line)
{
	const char *err_msg;
	const char *local_msg;

	if (error_buf[0] != 0 || (err_type < SPECASM_MAX_ERRORS))
		return;

	local_msg = exact_line ? "at" : "used by";
	switch (err_type) {
	case SALINK_ERROR_DIV_ZERO:
		err_msg = "Divide by zero";
		break;
	case SALINK_ERROR_RECURISVE_EQU:
		err_msg = "Recursive definition";
		break;
	case SALINK_ERROR_LOCAL_IN_GLOBAL_EQU:
		err_msg = "Global EQU references local EQU";
		break;
	case SALINK_ERROR_BAD_EXPRESSION:
		err_msg = "Bad expression";
		break;
	default:
		return;
	}
	snprintf(error_buf, sizeof(error_buf), "%s '%s' %s %s:%d", err_msg,
		 name, local_msg, g_obj->fname, line_no);
}

static void prv_check_equ_err(const char *name, const char *equ,
			      uint16_t line_no, uint8_t exact_line)
{
	snprintf(scratch, sizeof(scratch), "%s = %s", name, equ);
	prv_check_exp_err(scratch, line_no, exact_line);
}

static void prv_equ_eval_local_e(salink_label_t *label, uint8_t depth,
				 uint16_t line_no)
{
	const char *name;
	uint8_t id;
	const char *str;

	id = label->data.equ[1];
	name = salink_get_label_str_e(id, label->data.equ[0]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	label->data.off = prv_equ_eval_e(name, depth, 0, line_no);
	if (err_type >= SPECASM_MAX_ERRORS) {
		str = salink_get_label_str_e(label->id, label->type);
		prv_check_equ_err(str, name, line_no, 0);
		return;
	}

	if (label->type == SALINK_LABEL_TYPE_EQU_SHORT)
		label->type = SALINK_LABEL_TYPE_EQU_EVAL_SHORT;
	else
		label->type = SALINK_LABEL_TYPE_EQU_EVAL_LONG;
}

#ifdef SPECASM_NEXT_BANKED
void salink_equ_eval_global_banked_e(salink_obj_t *obj, salink_global_t *global,
				     salink_label_t *label, uint8_t depth)
#else
void salink_equ_eval_global_e(salink_obj_t *obj, salink_global_t *global,
			      salink_label_t *label, uint8_t depth)
#endif
{
	const char *name = global->name + strlen(global->name) + 1;

	g_obj = obj;
	label->data.off = prv_equ_eval_e(name, depth, 1, global->line_no);
	if (err_type >= SPECASM_MAX_ERRORS) {
		prv_check_equ_err(global->name, name, global->line_no, 1);
		return;
	}

	label->type = SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL;
}

static int16_t prv_eval_exp_from_id_e(salink_obj_t *obj, unsigned int line_no,
				      uint8_t id, uint8_t lng)
{
	const char *str;
	int16_t exp;

	str = salink_get_label_str_e(id, lng);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	g_obj = obj;
	exp = prv_equ_eval_e(str, 0, 0, line_no);
	prv_check_exp_err(str, line_no, 1);

	if ((err_type != SPECASM_ERROR_OK) && (err_type < SPECASM_MAX_ERRORS)) {
		snprintf(error_buf, sizeof(error_buf), "%s:%d %s", obj->fname,
			 line_no, specasm_error_msg(err_type));
		err_type = SALINK_ERROR_BAD_EXP;
	}

	return exp;
}

static int16_t prv_eval_equ_label(specasm_line_t *line, salink_obj_t *obj,
				  unsigned int line_no, uint8_t id)
{
	uint8_t label_type;
	uint8_t lng;

	label_type = specasm_line_get_addr_type(line);
	lng = label_type == SPECASM_FLAGS_ADDR_SHORT ? 0 : 1;
	return prv_eval_exp_from_id_e(obj, line_no, id, lng);
}

static void prv_eval_equ_8bit_e(specasm_line_t *line, salink_obj_t *obj,
				unsigned int line_no, uint8_t loc)
{
	int16_t exp;

	exp = prv_eval_equ_label(line, obj, line_no, line->data.op_code[loc]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((exp > 255) || (exp < -128)) {
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d immediate too big :%d", obj->fname, line_no,
			 exp);
		err_type = SALINK_ERROR_SIZE_TOO_BIG;
		return;
	}
	line->data.op_code[loc] = exp;
}

static void prv_eval_equ_16bit_e(specasm_line_t *line, salink_obj_t *obj,
				 unsigned int line_no, uint8_t loc)
{
	int16_t exp;

	exp = prv_eval_equ_label(line, obj, line_no, line->data.op_code[loc]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	*((uint16_t *)&line->data.op_code[loc]) = exp;
}

#ifdef SPECASM_TARGET_NEXT_OPCODES
static void prv_eval_equ_push_imm_e(specasm_line_t *line, salink_obj_t *obj,
				    unsigned int line_no)
{
	int16_t exp;
	uint8_t exp_rev[2];

	/*
	 * The push imm instruction is weird in that the immediate is encoded
	 * in big endian format.  If we're dealing with an expression this means
	 * that the expression id will be in byte 3 and not byte 2 as one might
	 * expect.
	 *
	 * We also need to make sure we reverse the bytes of the evaluated
	 * expression.
	 */

	exp = prv_eval_equ_label(line, obj, line_no, line->data.op_code[3]);
	if (err_type != SPECASM_ERROR_OK)
		return;
	memcpy(exp_rev, &exp, 2);
	line->data.op_code[2] = exp_rev[1];
	line->data.op_code[3] = exp_rev[0];
}
#endif

#ifdef SPECASM_NEXT_BANKED
void salink_apply_expressions_banked_e(specasm_line_t *line, salink_obj_t *obj,
				       unsigned int line_no)
#else
void salink_apply_expressions_e(specasm_line_t *line, salink_obj_t *obj,
				unsigned int line_no)
#endif
{
	int16_t exp;
	uint8_t opcode0;

	line->type -= SPECASM_LINE_TYPE_EXP_ADJ;

	switch (line->type) {
	case SPECASM_LINE_TYPE_ADC:
	case SPECASM_LINE_TYPE_ADD:
	case SPECASM_LINE_TYPE_AND:
	case SPECASM_LINE_TYPE_CP:
	case SPECASM_LINE_TYPE_IN:
	case SPECASM_LINE_TYPE_OUT:
	case SPECASM_LINE_TYPE_OR:
	case SPECASM_LINE_TYPE_SBC:
	case SPECASM_LINE_TYPE_SUB:
	case SPECASM_LINE_TYPE_XOR:
#ifdef SPECASM_TARGET_NEXT_OPCODES
		if ((line->type == SPECASM_LINE_TYPE_ADD) &&
		    (line->data.op_code[0] == 0xED))
			prv_eval_equ_16bit_e(line, obj, line_no, 2);
		else
			prv_eval_equ_8bit_e(line, obj, line_no, 1);
#else
		prv_eval_equ_8bit_e(line, obj, line_no, 1);
#endif
		break;
	case SPECASM_LINE_TYPE_RST:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[1]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if ((exp > 0x38) || (exp & 7)) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad argument to rst :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		line->data.op_code[0] |= exp;
		break;
	case SPECASM_LINE_TYPE_LD:
		opcode0 = line->data.op_code[0];
		if ((opcode0 & 0xC7) == 0x6) {
			prv_eval_equ_8bit_e(line, obj, line_no, 1);
			break;
		}
		switch (opcode0) {
		case 0x1:
		case 0x11:
		case 0x21:
		case 0x31:
		case 0x2a:
		case 0x3a:
		case 0x22:
		case 0x32:
			prv_eval_equ_16bit_e(line, obj, line_no, 1);
			break;
		case 0xDD:
		case 0xFD:
			if (line->data.op_code[1] == 0x36) {
				prv_eval_equ_8bit_e(line, obj, line_no, 3);
				break;
			}
		case 0xED:
			prv_eval_equ_16bit_e(line, obj, line_no, 2);
			break;
		}
		break;
	case SPECASM_LINE_TYPE_BIT:
	case SPECASM_LINE_TYPE_RES:
	case SPECASM_LINE_TYPE_SET:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[2]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (exp > 7) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad bit position :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		line->data.op_code[1] |= ((uint8_t)exp) << 3;
		break;
	case SPECASM_LINE_TYPE_IM:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[2]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (exp > 2) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad arg for im :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		if (exp == 2)
			line->data.op_code[1] = 0x5e;
		else if (exp == 1)
			line->data.op_code[1] = 0x56;
		break;
	case SPECASM_LINE_TYPE_CALL:
	case SPECASM_LINE_TYPE_JP:
		prv_eval_equ_16bit_e(line, obj, line_no, 1);
		break;
	case SPECASM_LINE_TYPE_DB:
		prv_eval_equ_8bit_e(line, obj, line_no, 0);
		break;
	case SPECASM_LINE_TYPE_DW:
		prv_eval_equ_16bit_e(line, obj, line_no, 0);
		break;
#ifdef SPECASM_TARGET_NEXT_OPCODES
	case SPECASM_LINE_TYPE_TEST:
	case SPECASM_LINE_TYPE_NEXTREG:
		prv_eval_equ_8bit_e(line, obj, line_no, 2);
		break;
	case SPECASM_LINE_TYPE_PUSH:
		prv_eval_equ_push_imm_e(line, obj, line_no);
		break;
#endif
	default:
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d unexpected expression", obj->fname, line_no);
		err_type = SALINK_ERROR_UNEXPECTED_EXP;
		break;
	}

	specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
}
