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

#include <stdlib.h>

#include "line.h"
#include "line_common.h"
#include "line_parse_common.h"
#include "state.h"

const char *specasm_parse_jump_label_e(const char *args, specasm_line_t *line,
				       uint8_t *label)
{
	uint8_t long_label;
	uint8_t i = 0;

	i = 0;
	while (*args != '-' && *args != ',' && *args != ' ' && *args != 0 &&
	       *args != ';' && *args != ')')
		scratch[i++] = *args++;
	scratch[i] = 0;
	specasm_state_check_label_e(scratch);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (i >= SPECASM_MAX_SHORT_LEN) {
		long_label = SPECASM_FLAGS_ADDR_LONG;
		*label = specasm_state_add_long_e(scratch);
	} else {
		long_label = SPECASM_FLAGS_ADDR_SHORT;
		*label = specasm_state_add_short_e(scratch);
	}
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	specasm_line_set_addr_type(line, long_label);

	return args;
}

const char *specasm_get_exp_e(specasm_line_t *line, const char *args,
			      uint8_t *val)
{
	uint8_t label_type;
	uint8_t read;

	read = specasm_parse_exp_e(args + 1, val, &label_type);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	specasm_line_set_addr_type(line, label_type);
	args += read + 1;
	line->type += SPECASM_LINE_TYPE_EXP_ADJ;
	return args;
}

const char *specasm_get_char_imm_e(const char *str, uint8_t *val,
				   uint8_t *flags)
{
	while (*str == ' ')
		++str;

	if (*str == '\'') {
		if (str[1] != '\'' && str[2] == '\'') {
			*val = str[1];
			*flags = SPECASM_FLAGS_NUM_CHAR;
			return (&str[3]);
		}
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	return NULL;
}

const char *specasm_get_word_imm_e(const char *str, uint16_t *val,
				   uint8_t *flags)
{
	const char *end_ptr;
	long lval;
	uint8_t bval;

	end_ptr = specasm_get_char_imm_e(str, &bval, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	if (end_ptr) {
		*val = (uint16_t)bval;
		return end_ptr;
	}

	end_ptr = specasm_get_long_imm_e(str, &lval, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	if ((lval > 0xffff) || (lval < -32768))
		goto on_error;
	*val = (uint16_t)lval;
	return end_ptr;

on_error:
	err_type = SPECASM_ERROR_NUM_TOO_BIG;
	return NULL;
}

const char *specasm_parse_word_imm_or_exp_e(const char *args,
					    specasm_line_t *line, uint16_t *val,
					    uint8_t *flags)
{
	uint8_t label;

	while (*args == ' ')
		++args;

	if (*args == '=') {
		args = specasm_get_exp_e(line, args, &label);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
		*val = label;
		*flags = 0;
		return args;
	}

	args = specasm_get_word_imm_e(args, val, flags);
	return args;
}

const char *specasm_parse_label_or_exp_e(const char *args, specasm_line_t *line,
					 uint8_t *label)
{
	const char *args2;

	if (*args == '=')
		args2 = specasm_get_exp_e(line, args, label);
	else
		args2 = specasm_parse_jump_label_e(args, line, label);

	return args2;
}

char *specasm_get_uword_imm_e(const char *str, uint16_t *val, uint8_t *flags)
{
	char *end_ptr;
	long lval;

	end_ptr = specasm_get_long_imm_e(str, &lval, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	if (*flags == SPECASM_FLAGS_NUM_SIGNED) {
		err_type = SPECASM_ERROR_NUM_NEG;
		return NULL;
	}
	if (lval > 0xffff) {
		err_type = SPECASM_ERROR_NUM_TOO_BIG;
		return NULL;
	}

	*val = (uint16_t)lval;
	return end_ptr;
}

char *specasm_get_long_imm_e(const char *str, long *val, uint8_t *flags)
{
	int base = 10;
	char *end_ptr;

	while (*str == ' ')
		++str;

	if (*str == '$') {
		base = 16;
		++str;
	}

	*val = strtol((char *)str, &end_ptr, base);
	if (end_ptr == (char *)str)
		goto on_error;
	if (base == 16) {
		if (*val < 0)
			goto on_error;
		*flags = SPECASM_FLAGS_NUM_HEX;
	} else {
		if (*val < 0)
			*flags = SPECASM_FLAGS_NUM_SIGNED;
		else
			*flags = SPECASM_FLAGS_NUM_UNSIGNED;
	}

	return end_ptr;

on_error:
	err_type = SPECASM_ERROR_BAD_NUM;
	return NULL;
}

static char *prv_get_offset_imm_e(const char *str, uint8_t *val, uint8_t *flags)
{
	char *end_ptr;
	long lval;

	end_ptr = specasm_get_long_imm_e(str, &lval, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	if (*flags == SPECASM_FLAGS_NUM_HEX) {
		if (lval > 255)
			goto on_error;
	} else {
		if ((lval < -128) || (lval > 127))
			goto on_error;
	}
	*val = (uint8_t)lval;
	return end_ptr;

on_error:
	err_type = SPECASM_ERROR_NUM_TOO_BIG;
	return NULL;
}

uint8_t specasm_parse_reg_e(const char *str, uint8_t *r, uint8_t *off,
			    uint8_t *flags)
{
	uint8_t reg;
	uint8_t i;
	const char *start = str;

	while (*str == ' ')
		++str;

	if (*str == 0) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	/*
	 * Check for indirect registers.
	 */

	reg = *str++;
	if (reg == '(') {
		while (*str == ' ')
			++str;
		if (!*str) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		if (*str == 'h' && *(str + 1) == 'l') {
			reg = SPECASM_BYTE_REG_HL_IND;
			str += 2;
		} else if (*str == 'b' && *(str + 1) == 'c') {
			reg = SPECASM_BYTE_REG_BC_IND;
			str += 2;
		} else if (*str == 'd' && *(str + 1) == 'e') {
			reg = SPECASM_BYTE_REG_DE_IND;
			str += 2;
		} else if (*str == 's' && *(str + 1) == 'p') {
			reg = SPECASM_BYTE_REG_SP_IND;
			str += 2;
		} else if (*str == 'i') {
			++str;
			if (*str == 'x') {
				reg = SPECASM_BYTE_REG_IX_OFF;
			} else if (*str == 'y') {
				reg = SPECASM_BYTE_REG_IY_OFF;
			} else {
				err_type = SPECASM_ERROR_BAD_REG;
				return 0;
			}
			++str;
			while (*str == ' ')
				++str;

			if (*str == ')') {
				reg += 2;
				str++;
				goto finish;
			}

			if (*str++ != '+') {
				err_type = SPECASM_ERROR_BAD_REG;
				return 0;
			}
			str = prv_get_offset_imm_e(str, off, flags);
			if (err_type != SPECASM_ERROR_OK)
				return 0;
		} else if (str[1] == ' ' || str[1] == ')') {
			for (i = 0; i < sizeof(byte_regs); i++)
				if (byte_regs[i] == *str)
					break;
			if (i == sizeof(byte_regs)) {
				err_type = SPECASM_ERROR_BAD_REG;
				return 0;
			}
			++str;
			reg = i + SPECASM_IND_MOD;
		}
		while (*str == ' ')
			++str;
		if (*str++ != ')') {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}

		goto finish;
	}

	/* Check for byte reg */

	if (*str == 0 || *str == ' ' || *str == ';' || *str == ',') {
		for (i = 0; i < sizeof(byte_regs); i++)
			if (byte_regs[i] == reg)
				break;

		if (i < sizeof(byte_regs)) {
			reg = i;
		} else if (reg == 'i') {
			reg = SPECASM_BYTE_REG_I;
		} else if (reg == 'r') {
			reg = SPECASM_BYTE_REG_R;
		} else {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		goto finish;
	}

	/* We must have a 16 bit reg */

	if (reg == 'h' && *str == 'l') {
		reg = SPECASM_BYTE_REG_HL;
	} else if (reg == 'd' && *str == 'e') {
		reg = SPECASM_BYTE_REG_DE;
	} else if (reg == 'b' && *str == 'c') {
		reg = SPECASM_BYTE_REG_BC;
	} else if (reg == 's' && *str == 'p') {
		reg = SPECASM_BYTE_REG_SP;
	} else if (reg == 'a' && *str == 'f') {
		if (str[1] == '\'') {
			reg = SPECASM_BYTE_REG_AF_P;
			str++;
		} else {
			reg = SPECASM_BYTE_REG_AF;
		}
	} else if (reg == 'i') {
		if (*str == 'y') {
			reg = SPECASM_BYTE_REG_IY;
		} else if (*str == 'x') {
			reg = SPECASM_BYTE_REG_IX;
		} else {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
	} else {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}
	str++;
	if (*str != ' ' && *str != ',' && *str != 0 && *str != ';') {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

finish:
	*r = reg;
	return (uint8_t)(str - start);
}

/*
 * The following functions are only used by line_parse.c but don't fit
 * in the 8Kb page.
 */

const char *specasm_get_byte_imm_e(const char *str, uint8_t *val,
				   uint8_t *flags)
{
	const char *end_ptr;
	long lval;

	end_ptr = specasm_get_char_imm_e(str, val, flags);
	if (end_ptr || err_type != SPECASM_ERROR_OK)
		return end_ptr;

	end_ptr = specasm_get_long_imm_e(str, &lval, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	if (lval > 255 || lval < -128)
		goto on_error;
	*val = (uint8_t)lval;
	return end_ptr;

on_error:
	err_type = SPECASM_ERROR_NUM_TOO_BIG;
	return NULL;
}

const char *specasm_get_byte_imm_ind_e(specasm_line_t *line, const char *args,
				       uint8_t *val)
{
	long v;
	uint8_t flags;
	uint8_t label;

	while (*args == ' ')
		++args;

	if (*args != '(') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	++args;
	while (*args == ' ')
		++args;

	if (*args == '=') {
		args = specasm_get_exp_e(line, args, &label);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
		v = label;
	} else {
		args = specasm_get_long_imm_e(args, &v, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
		if (flags == SPECASM_FLAGS_NUM_SIGNED) {
			err_type = SPECASM_ERROR_NUM_NEG;
			return NULL;
		}
		if (v > 0xff) {
			err_type = SPECASM_ERROR_NUM_TOO_BIG;
			return NULL;
		}
		specasm_line_set_format(line, flags);
	}

	while (*args == ' ')
		++args;

	if (*args != ')') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}
	*val = v;

	return args + 1;
}

const char *specasm_parse_byte_imm_or_exp_e(const char *args,
					    specasm_line_t *line, uint8_t *val,
					    uint8_t *label)
{
	uint8_t flags;

	while (*args == ' ')
		++args;

	if (*args == '=') {
		args = specasm_get_exp_e(line, args, label);
		if (err_type != SPECASM_ERROR_OK)
			return NULL;
		if (val != label)
			*val = 0;
		return args;
	}

	args = specasm_get_byte_imm_e(args, val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	specasm_line_set_format(line, flags);

	return args;
}

const char *specasm_parse_reg_comma_e(const char *args, uint8_t *r,
				      uint8_t *off, uint8_t *flags)
{
	const char *args2;

	args2 = args + specasm_parse_reg_e(args, r, off, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (*args2 == ' ')
		++args2;

	if (*args2 != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return NULL;
	}

	return args2 + 1;
}
