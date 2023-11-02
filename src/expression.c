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

struct salink_token_t_ {
	uint8_t type;
	union {
		int16_t num;
		uint8_t id;
	} data;
};

typedef struct salink_token_t_ salink_token_t;

static const char *prv_exp_priority4_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no);

static void prv_equ_eval_local_e(salink_obj_t *obj, salink_label_t *label,
				 uint8_t depth, uint16_t line_no);

const char *prv_get_token_e(const char *buf, salink_obj_t *obj,
			    salink_token_t *tok, uint8_t is_global)
{
	const char *start;
	char *end_ptr;
	uint8_t i;
	char c;
	long lval;
	int len;
#ifndef SPECASM_NEXT_BANKED
	const char simple_ops[] = "()/*+-&|^~";
#else
	static const char simple_ops[] = "()/*+-&|^~";
#endif
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
			tok->data.id = salink_find_global_label_e(scratch, obj);
			tok->type = SALINK_TOKEN_GLOBAL_LABEL;
		} else {
			tok->data.id =
			    salink_find_local_label_e(scratch, len, obj);
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

static const char *prv_exp_priority0_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no)
{
	salink_token_t tok;
	salink_label_t *label;
	salink_global_t *global;

	str = prv_get_token_e(str, obj, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK) {
		return NULL;
	}

	switch (tok.type) {
	case SALINK_TOKEN_OP:
		str =
		    prv_exp_priority4_e(str, obj, e, depth, is_global, line_no);
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
			str = prv_get_token_e(str, obj, &tok, is_global);
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
			if (depth == 8) {
				err_type = SALINK_ERROR_RECURISVE_EQU;
				return NULL;
			}
			prv_equ_eval_local_e(obj, label, depth + 1, line_no);
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
			if (depth == 8) {
				err_type = SALINK_ERROR_RECURISVE_EQU;
				return NULL;
			}
			salink_equ_eval_global_e(obj, global, label, depth + 1);
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

static const char *prv_exp_priority1_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;

	str = prv_exp_priority0_e(str, obj, &e1, depth, is_global, line_no);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, obj, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '*' && op != '/' && op != '%')
			break;

		str = prv_exp_priority0_e(next, obj, &e2, depth, is_global,
					  line_no);
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
		next = prv_get_token_e(str, obj, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority2_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;

	str = prv_exp_priority1_e(str, obj, &e1, depth, is_global, line_no);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, obj, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '+' && op != '-')
			break;

		str = prv_exp_priority1_e(next, obj, &e2, depth, is_global,
					  line_no);
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
		next = prv_get_token_e(str, obj, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority3_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;

	str = prv_exp_priority2_e(str, obj, &e1, depth, is_global, line_no);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, obj, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if ((op != SALINK_TOKEN_LSL_VAL) &&
		    (op != SALINK_TOKEN_ASR_VAL))
			break;

		str = prv_exp_priority2_e(next, obj, &e2, depth, is_global,
					  line_no);
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
		next = prv_get_token_e(str, obj, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static const char *prv_exp_priority4_e(const char *str, salink_obj_t *obj,
				       int16_t *e, uint8_t depth,
				       uint8_t is_global, uint16_t line_no)
{
	salink_token_t tok;
	int16_t e1;
	int16_t e2;
	char op;
	const char *next;

	str = prv_exp_priority3_e(str, obj, &e1, depth, is_global, line_no);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	next = prv_get_token_e(str, obj, &tok, is_global);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (tok.type == SALINK_TOKEN_OP) {
		op = (char)tok.data.id;
		if (op != '&' && op != '|' && op != '^')
			break;

		str = prv_exp_priority3_e(next, obj, &e2, depth, is_global,
					  line_no);
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
		next = prv_get_token_e(str, obj, &tok, is_global);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
	}

	*e = e1;

	return str;
}

static int16_t prv_equ_eval_e(salink_obj_t *obj, const char *name,
			      uint8_t depth, uint8_t is_global,
			      uint16_t line_no)
{
	const char *str;
	int16_t e;
	salink_token_t tok;

	str = prv_exp_priority4_e(name, obj, &e, depth, is_global, line_no);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	(void)prv_get_token_e(str, obj, &tok, is_global);
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

static void prv_check_exp_err(salink_obj_t *obj, const char *name,
			      uint16_t line_no, uint8_t exact_line)
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
		 name, local_msg, obj->fname, line_no);
}

static void prv_check_equ_err(salink_obj_t *obj, const char *name,
			      const char *equ, uint16_t line_no,
			      uint8_t exact_line)
{
	snprintf(scratch, sizeof(scratch), "%s = %s", name, equ);
	prv_check_exp_err(obj, scratch, line_no, exact_line);
}

#ifdef SPECASM_NEXT_BANKED
int16_t salink_equ_eval_banked_e(salink_obj_t *obj, const char *str,
				 uint16_t line_no)
#else
int16_t salink_equ_eval_e(salink_obj_t *obj, const char *str, uint16_t line_no)
#endif
{
	int16_t e;

	e = prv_equ_eval_e(obj, str, 0, 0, line_no);
	prv_check_exp_err(obj, str, line_no, 1);

	return e;
}

static void prv_equ_eval_local_e(salink_obj_t *obj, salink_label_t *label,
				 uint8_t depth, uint16_t line_no)
{
	const char *name;
	uint8_t id;
	const char *str;

	id = label->data.equ[1];
	name = salink_get_label_str_e(id, label->data.equ[0]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	label->data.off = prv_equ_eval_e(obj, name, depth, 0, line_no);
	if (err_type >= SPECASM_MAX_ERRORS) {
		str = salink_get_label_str_e(label->id, label->type);
		prv_check_equ_err(obj, str, name, line_no, 0);
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

	label->data.off = prv_equ_eval_e(obj, name, depth, 1, global->line_no);
	if (err_type >= SPECASM_MAX_ERRORS) {
		prv_check_equ_err(obj, global->name, name, global->line_no, 1);
		return;
	}

	label->type = SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL;
}
