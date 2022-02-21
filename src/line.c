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
#include <string.h>

#include "peer.h"
#include "state.h"

#define SPECASM_BYTE_REG_B 0
#define SPECASM_BYTE_REG_C 1
#define SPECASM_BYTE_REG_D 2
#define SPECASM_BYTE_REG_E 3
#define SPECASM_BYTE_REG_H 4
#define SPECASM_BYTE_REG_L 5
#define SPECASM_BYTE_REG_A 7
#define SPECASM_BYTE_REG_BC 8
#define SPECASM_BYTE_REG_DE 9
#define SPECASM_BYTE_REG_HL 10
#define SPECASM_BYTE_REG_AF 11
#define SPECASM_BYTE_REG_SP 12
#define SPECASM_BYTE_REG_IX 13
#define SPECASM_BYTE_REG_IY 14
#define SPECASM_BYTE_REG_AF_P 15
#define SPECASM_BYTE_REG_IX_OFF 16
#define SPECASM_BYTE_REG_IY_OFF 17
#define SPECASM_BYTE_REG_IX_IND 18 // IX_IND and IY_IND need to be adjacent.
#define SPECASM_BYTE_REG_IY_IND 19 // to IX_OFF and IY_OFF
#define SPECASM_BYTE_REG_I 20
#define SPECASM_BYTE_REG_R 21

#define SPECASM_IND_MOD 32
#define SPECASM_BYTE_REG_B_IND (SPECASM_BYTE_REG_B + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_C_IND (SPECASM_BYTE_REG_C + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_D_IND (SPECASM_BYTE_REG_D + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_E_IND (SPECASM_BYTE_REG_E + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_H_IND (SPECASM_BYTE_REG_H + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_L_IND (SPECASM_BYTE_REG_L + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_A_IND (SPECASM_BYTE_REG_A + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_SP_IND (SPECASM_BYTE_REG_SP + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_BC_IND (SPECASM_BYTE_REG_BC + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_DE_IND (SPECASM_BYTE_REG_DE + SPECASM_IND_MOD)
#define SPECASM_BYTE_REG_HL_IND (SPECASM_BYTE_REG_HL + SPECASM_IND_MOD)

#define SPECASM_CC_NZ 0
#define SPECASM_CC_Z 1
#define SPECASM_CC_NC 2
#define SPECASM_CC_C 3
#define SPECASM_CC_PO 4
#define SPECASM_CC_PE 5
#define SPECASM_CC_P 6
#define SPECASM_CC_M 7
#define SPECASM_CC_NONE 8

typedef uint8_t (*specasm_dump_opcode_fn_t)(const specasm_line_t *line,
					    char *buf);

struct specasm_line_opcode_dump_t_ {
	specasm_dump_opcode_fn_t fn;
	const char name[4];
};

typedef struct specasm_line_opcode_dump_t_ specasm_line_opcode_dump_t;

typedef struct specasm_opcode_t_ specasm_opcode_t;

typedef uint8_t (*specasm_parse_fn_t)(const char *args, specasm_line_t *line,
				      const specasm_opcode_t *op_code);

struct specasm_opcode_t_ {
	const char mnemomic[SPECASM_MAX_MNEMOM + 1];
	specasm_parse_fn_t fn;
	uint8_t line_type;
	uint8_t op_code[2];
};

static char byte_regs[] = {'b', 'c', 'd', 'e', 'h', 'l', ' ', 'a'};

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

static const char *prv_get_char_imm_e(const char *str, uint8_t *val,
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

static const char *prv_get_byte_imm_e(const char *str, uint8_t *val,
				      uint8_t *flags)
{
	const char *end_ptr;
	long lval;

	end_ptr = prv_get_char_imm_e(str, val, flags);
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

static const char *prv_get_word_imm_e(const char *str, uint16_t *val,
				      uint8_t *flags)
{
	const char *end_ptr;
	long lval;
	uint8_t bval;

	end_ptr = prv_get_char_imm_e(str, &bval, flags);
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

static char *prv_get_uword_imm_e(const char *str, uint16_t *val, uint8_t *flags)
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

static const char *prv_get_word_imm_ind_e(const char *args, uint16_t *val,
					  uint8_t *flags)
{
	while (*args == ' ')
		++args;

	if (*args != '(') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}
	args = prv_get_uword_imm_e(args + 1, val, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	while (*args == ' ')
		++args;

	if (*args != ')') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	return args + 1;
}

static const char *prv_get_byte_imm_ind_e(const char *args, uint8_t *val,
					  uint8_t *flags)
{
	long v;

	while (*args == ' ')
		++args;

	if (*args != '(') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	args = specasm_get_long_imm_e(args + 1, &v, flags);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	if (*flags == SPECASM_FLAGS_NUM_SIGNED) {
		err_type = SPECASM_ERROR_NUM_NEG;
		return NULL;
	}
	if (v > 0xff) {
		err_type = SPECASM_ERROR_NUM_TOO_BIG;
		return NULL;
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

static uint8_t prv_parse_reg_e(const char *str, uint8_t *r, uint8_t *off,
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

static const char *prv_parse_reg_comma_e(const char *args, uint8_t *r,
					 uint8_t *off, uint8_t *flags)
{
	const char *args2;

	args2 = args + prv_parse_reg_e(args, r, off, flags);
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

static uint8_t prv_dump_byte(char *buf, uint8_t v, uint8_t flags)
{
	int iv;
	unsigned char radix;
	const char *start;

	if (flags == SPECASM_FLAGS_NUM_CHAR) {
		buf[0] = '\'';
		buf[1] = v;
		buf[2] = '\'';
		buf[3] = 0;
		return 3;
	}

	start = buf;
	if (flags == SPECASM_FLAGS_NUM_HEX) {
		*buf++ = '$';
		radix = 16;
		iv = v;
	} else {
		radix = 10;
		if (flags == SPECASM_FLAGS_NUM_SIGNED)
			iv = (int8_t)v;
		else
			iv = v;
	}
	(void)itoa(iv, buf, radix);
	return strlen(start);
}

static uint8_t prv_dump_word(char *buf, uint16_t v, uint8_t flags)
{
	const char *start;
	unsigned char radix;

	if (flags == SPECASM_FLAGS_NUM_CHAR) {
		buf[0] = '\'';
		buf[1] = (uint8_t)v;
		buf[2] = '\'';
		buf[3] = 0;
		return 3;
	}

	start = buf;
	if (flags == SPECASM_FLAGS_NUM_SIGNED) {
		(void)itoa(v, buf, 10);
	} else {
		if (flags == SPECASM_FLAGS_NUM_HEX) {
			*buf++ = '$';
			radix = 16;
		} else {
			radix = 10;
		}

		(void)utoa(v, buf, radix);
	}

	return strlen(start);
}

static char *prv_dump_index(const uint8_t *op_code, char *buf, uint8_t flags)
{
	buf[0] = '(';
	buf[1] = 'i';
	buf[2] = op_code[0] == 0xDD ? 'x' : 'y';
	buf[3] = '+';
	buf += 4;
	buf += prv_dump_byte(buf, op_code[2], flags);
	*buf++ = ')';
	return buf;
}

static char *prv_byte_and_hl_ind(uint8_t code, char *buf)
{
	code &= 7;
	if (code == 6) {
		buf[0] = '(';
		buf[1] = 'h';
		buf[2] = 'l';
		buf[3] = ')';
		buf += 4;
	} else {
		*buf++ = byte_regs[code];
	}
	return buf;
}

static char *prv_dump_arith_e(const specasm_line_t *line, char *buf,
			      uint8_t imm, uint8_t reg)
{
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0xDD || op_code[0] == 0xFD) {
		buf =
		    prv_dump_index(op_code, buf, specasm_line_get_format(line));
	} else if (op_code[0] == imm) {
		buf += prv_dump_byte(buf, op_code[1],
				     specasm_line_get_format(line));
	} else if ((op_code[0] & reg) == reg) {
		buf = prv_byte_and_hl_ind(op_code[0], buf);
	} else {
		err_type = SPECASM_ERROR_BAD_REG;
	}

	return buf;
}

static uint8_t prv_dump_fixed_e(uint8_t op_code, char *buf, const char **com,
				uint8_t count)
{
	uint8_t i;
	const char *p;
	uint8_t byte0;
	char *start = buf;

	for (i = 0; i < count; i++) {
		byte0 = (uint8_t)com[i][0];
		if (byte0 == op_code) {
			p = com[i] + 1;
			while (*p)
				*buf++ = *p++;
			return buf - start;
		}
	}

	err_type = SPECASM_ERROR_BAD_MNENOMIC;
	return 0;
}

static char *prv_dump_16bit_reg(char *buf, uint8_t code)
{
	const char *p;
	const char *sixbit[] = {"bc", "de", "hl", "sp"};

	p = sixbit[(code >> 4) & 3];
	while (*p)
		*buf++ = *p++;

	return buf;
}

static char *prv_dump_16bit_unary_e(const specasm_line_t *line, char *buf,
				    uint8_t rind, uint8_t hl_ind)
{
	uint8_t reg;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0xFD || op_code[0] == 0xDD) {
		if (op_code[1] == hl_ind) {
			buf = prv_dump_index(op_code, buf,
					     specasm_line_get_format(line));
		} else {
			*buf++ = 'i';
			*buf++ = (op_code[0] == 0xFD) ? 'y' : 'x';
		}
	} else if ((op_code[0] & 0xF) == rind) {
		buf = prv_dump_16bit_reg(buf, op_code[0]);
	} else {
		reg = (op_code[0] == hl_ind) ? 6 : op_code[0] >> 3;
		buf = prv_byte_and_hl_ind(reg, buf);
	}

	return buf;
}

static char *prv_dump_jump_label_fmt_e(const specasm_line_t *line, char *buf,
				       uint16_t id, uint8_t fmt)
{
	const char *label;

	if (fmt == SPECASM_FLAGS_ADDR_NUM) {
		buf += prv_dump_word(buf, id, specasm_line_get_format(line));
	} else {
		if (fmt == SPECASM_FLAGS_ADDR_LONG)
			label = specasm_state_get_long_e((uint8_t)id);
		else
			label = specasm_state_get_short_e((uint8_t)id);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		while (*label)
			*buf++ = *label++;
	}

	return buf;
}

static char *prv_dump_jump_label_e(const specasm_line_t *line, char *buf,
				   uint16_t id)
{
	uint8_t fmt = specasm_line_get_addr_type(line);
	return prv_dump_jump_label_fmt_e(line, buf, id, fmt);
}

static char *prv_dump_cc(const uint8_t cc, char *buf)
{
	static const char *codes[] = {"nz", "z",  "nc", "c",
				      "po", "pe", "p",  "m"};

	const char *code = codes[cc & 7];
	while (*code)
		*buf++ = *code++;

	return buf;
}

static uint8_t prv_dump_adc_sbc_e(const specasm_line_t *line, char *buf,
				  uint8_t aimm, uint8_t hl_ind)
{
	char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0xED) {
		buf[0] = 'h';
		buf[1] = 'l';
		buf[2] = ',';
		buf[3] = ' ';
		buf = prv_dump_16bit_reg(buf + 4, op_code[1]);
	} else {
		buf[0] = 'a';
		buf[1] = ',';
		buf[2] = ' ';
		buf = prv_dump_arith_e(line, buf + 3, aimm, hl_ind);
	}

	return buf - start;
}

static uint8_t prv_dump_adc_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_adc_sbc_e(line, buf, 0xCE, 0x88);
}

static uint8_t prv_dump_add_e(const specasm_line_t *line, char *buf)
{
	uint8_t reg1;
	uint8_t reg2;
	const char *code;
	static const char *codes[] = {"bc", "de", "hl", "sp", "ix", "iy"};
	const uint8_t *op_code = line->data.op_code;

	char *start = buf;

	if ((op_code[0] & 0xC9) == 0x9) {
		reg1 = 2;
		reg2 = op_code[0] >> 4;
	} else if ((op_code[0] == 0xFD) && ((op_code[1] & 0xC9) == 0x9)) {
		reg1 = 5;
		reg2 = (op_code[1] >> 4) & 3;
		if (reg2 == 2)
			reg2 = 5;
	} else if ((op_code[0] == 0xDD) && ((op_code[1] & 0xC9) == 0x9)) {
		reg1 = 4;
		reg2 = (op_code[1] >> 4) & 3;
		if (reg2 == 2)
			reg2 = 4;
	} else {
		buf[0] = 'a';
		buf[1] = ',';
		buf[2] = ' ';

		return prv_dump_arith_e(line, buf + 3, 0xC6, 0x80) - start;
	}

	code = codes[reg1];
	while (*code)
		*buf++ = *code++;
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;
	code = codes[reg2];
	while (*code)
		*buf++ = *code++;
	return buf - start;
}

static uint8_t prv_dump_and_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xE6, 0xA0) - start;
}

static uint8_t prv_dump_bit_e(const specasm_line_t *line, char *buf)
{
	uint8_t val;
	char *start = buf;
	const uint8_t *op_code = line->data.op_code;
	uint8_t index = op_code[0] == 0xDD || op_code[0] == 0xFD;

	val = (index) ? op_code[3] : op_code[1];

	(void)itoa((val >> 3) & 7, buf, 10);
	buf += strlen(buf);
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	if (op_code[0] == 0xCB)
		buf = prv_byte_and_hl_ind(op_code[1], buf);
	else
		buf =
		    prv_dump_index(op_code, buf, specasm_line_get_format(line));

	return buf - start;
}

static uint8_t prv_dump_jr_call_e(const specasm_line_t *line, char *buf,
				  uint8_t abs_ind, uint8_t mask)
{
	uint16_t val;
	uint8_t fmt = specasm_line_get_addr_type(line);
	const char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] != abs_ind) {
		buf = prv_dump_cc((op_code[0] >> 3) & mask, buf);
		buf[0] = ',';
		buf[1] = ' ';
		buf += 2;
	}

	if (fmt == SPECASM_FLAGS_ADDR_NUM) {
		val = *((uint16_t *)&op_code[1]);
		buf += prv_dump_word(buf, val, specasm_line_get_format(line));
	} else {
		buf = prv_dump_jump_label_e(line, buf, op_code[1]);
	}
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return buf - start;
}

static uint8_t prv_dump_call_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_jr_call_e(line, buf, 0xcd, 7);
}

static uint8_t prv_dump_cp_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xFE, 0xB8) - start;
}

static uint8_t prv_dump_dec_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;
	return prv_dump_16bit_unary_e(line, buf, 0xB, 0x35) - start;
}

static uint8_t prv_dump_djnz_e(const specasm_line_t *line, char *buf)
{
	const char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	buf = prv_dump_jump_label_e(line, buf, op_code[1]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	return buf - start;
}

static uint8_t prv_dump_ex_e(const specasm_line_t *line, char *buf)
{
	static const char *com[] = {
	    "\x08"
	    "af, af'",
	    "\xeb"
	    "de, hl",
	    "\xe3"
	    "(sp), hl",
	    "\xdd"
	    "(sp), ix",
	    "\xfd"
	    "(sp), iy",
	};
	const uint8_t *op_code = line->data.op_code;

	return prv_dump_fixed_e(op_code[0], buf, com,
				sizeof(com) / sizeof(char *));
}

static uint8_t prv_dump_im_e(const specasm_line_t *line, char *buf)
{
	char ch;
	const uint8_t *op_code = line->data.op_code;

	switch (op_code[1]) {
	case 0x46:
		ch = '0';
		break;
	case 0x56:
		ch = '1';
		break;
	case 0x5E:
		ch = '2';
		break;
	default:
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	*buf++ = ch;
	return 1;
}

static uint8_t prv_dump_in_e(const specasm_line_t *line, char *buf)
{
	uint8_t code;
	char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	code =
	    (op_code[0] == 0xDB) ? SPECASM_BYTE_REG_A : (op_code[1] >> 3) & 7;

	buf[0] = byte_regs[code];
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = '(';
	buf += 4;
	if (op_code[0] == 0xDB) {
		buf += prv_dump_byte(buf, op_code[1],
				     specasm_line_get_format(line));
	} else {
		*buf++ = 'c';
	}
	buf[0] = ')';

	return (buf + 1) - start;
}

static uint8_t prv_dump_inc_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;
	return prv_dump_16bit_unary_e(line, buf, 0x3, 0x34) - start;
}

static uint8_t prv_dump_jp_e(const specasm_line_t *line, char *buf)
{
	const char *start = buf;
	static const char *com[] = {
	    "\xe9"
	    "(hl)",
	    "\xdd"
	    "(ix)",
	    "\xfd"
	    "(iy)",
	};
	const uint8_t *op_code = line->data.op_code;

	switch (op_code[0]) {
	case 0xDD:
	case 0xFD:
	case 0xE9:
		return prv_dump_fixed_e(op_code[0], buf, com,
					sizeof(com) / sizeof(uint8_t *));
	default:
		if (op_code[0] != 0xC3) {
			buf = prv_dump_cc(op_code[0] >> 3, buf);
			buf[0] = ',';
			buf[1] = ' ';
			buf += 2;
		} else if ((op_code[0] & 0xC2) != 0xC2) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}

		buf = prv_dump_jump_label_e(line, buf, op_code[1]);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
	}

	return buf - start;
}

static uint8_t prv_dump_jr_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_jr_call_e(line, buf, 0x18, 3);
}

static char *prv_dump_label_ind_e(const specasm_line_t *line, char *buf,
				  uint8_t opcode0)
{
	uint16_t id;
	uint8_t i = 2;

	if ((opcode0 == 0x2A) || (opcode0 == 0x3A))
		i = 1;

	*buf++ = '(';
	/*
	 * id could be a label (1 byte) or a 16 bit word.
	 */
	id = *((uint16_t *)&line->data.op_code[i]);
	buf = prv_dump_jump_label_e(line, buf, id);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	*buf++ = ')';

	return buf;
}

static char *prv_dump_label_ind_reg_e(const specasm_line_t *line, char *buf,
				      uint8_t opcode0)
{
	static const char *com[] = {
	    "\x3a"
	    "a",
	    "\xed"
	    "bc",
	    "\xee"
	    "de",
	    "\xf0"
	    "sp",
	    "\x2a"
	    "hl",
	    "\xdd"
	    "ix",
	    "\xfd"
	    "iy",
	};

	if (opcode0 == 0xED)
		opcode0 += (line->data.op_code[1] >> 4) & 3;

	return buf + prv_dump_fixed_e(opcode0, buf, com,
				      sizeof(com) / sizeof(uint8_t *));
}

static uint8_t prv_dump_ld_ind_imm_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;
	uint8_t opcode0 = line->data.op_code[0] | 8;

	buf = prv_dump_label_ind_e(line, buf, opcode0);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	buf[0] = ',';
	buf[1] = ' ';

	return prv_dump_label_ind_reg_e(line, buf + 2, opcode0) - start;
}

static uint8_t prv_dump_ld_ind_imm_rev_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;
	uint8_t opcode0 = line->data.op_code[0];

	buf = prv_dump_label_ind_reg_e(line, buf, opcode0);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	buf[0] = ',';
	buf[1] = ' ';

	return prv_dump_label_ind_e(line, buf + 2, opcode0) - start;
}

static char *prv_dump_ld_16bit_imm_e(const specasm_line_t *line, char *buf,
				     uint8_t reg, uint16_t val)
{
	const char *code;
	static const char *codes[] = {"bc", "de", "hl",  "sp",
				      "ix", "iy", "(hl)"};

	code = codes[reg];
	while (*code)
		*buf++ = *code++;

	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;
	if (specasm_line_get_addr_type(line)) {
		buf = prv_dump_jump_label_e(line, buf, val);
	} else {
		buf += prv_dump_word(buf, val, specasm_line_get_format(line));
	}

	return buf;
}

static char *prv_dump_st_reg(uint8_t opcode0, char *buf)
{
	uint8_t reg = SPECASM_BYTE_REG_A;

	buf[0] = '(';
	if (opcode0 == 0x2) {
		buf[1] = 'b';
		buf[2] = 'c';
	} else if (opcode0 == 0x12) {
		buf[1] = 'd';
		buf[2] = 'e';
	} else {
		buf[1] = 'h';
		buf[2] = 'l';
		reg = opcode0 & 7;
	}
	buf[3] = ')';
	buf[4] = ',';
	buf[5] = ' ';
	buf[6] = byte_regs[reg];
	return buf + 7;
}

static char *prv_dump_ld_8bit_reg(uint8_t opcode0, char *buf)
{
	buf[0] = byte_regs[(opcode0 >> 3) & 7];
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = byte_regs[opcode0 & 7];

	return buf + 4;
}

static char *prv_dump_lda_ind(uint8_t opcode0, char *buf)
{
	uint8_t a, b;

	if (opcode0 == 0xA) {
		a = 'b';
		b = 'c';
	} else {
		a = 'd';
		b = 'e';
	}
	buf[0] = 'a';
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = '(';
	buf[4] = a;
	buf[5] = b;
	buf[6] = ')';

	return buf + 7;
}

static char *prv_dump_ld_hl_ind_e(uint8_t opcode0, char *buf)
{
	static const char *com[] = {
	    "\x46"
	    "b",
	    "\x4E"
	    "c",
	    "\x56"
	    "d",
	    "\x5E"
	    "e",
	    "\x66"
	    "h",
	    "\x6E"
	    "l",
	    "\x7E"
	    "a",
	};

	buf +=
	    prv_dump_fixed_e(opcode0, buf, com, sizeof(com) / sizeof(char *));
	if (err_type != SPECASM_ERROR_OK)
		return NULL;
	buf[0] = ',';
	buf[1] = ' ';
	buf[2] = '(';
	buf[3] = 'h';
	buf[4] = 'l';
	buf[5] = ')';
	return buf + 6;
}

static char *prv_dump_ld_reg_off_ind(const specasm_line_t *line, char *buf)
{
	buf[0] = byte_regs[(line->data.op_code[1] >> 3) & 7];
	buf[1] = ',';
	buf[2] = ' ';
	return prv_dump_index(line->data.op_code, buf + 3,
			      specasm_line_get_format(line));
}

static char *prv_dump_ld_ind_off_reg(const specasm_line_t *line, char *buf)
{
	buf = prv_dump_index(line->data.op_code, buf,
			     specasm_line_get_format(line));
	buf[0] = ',';
	buf[1] = ' ';
	buf[2] = byte_regs[line->data.op_code[1] & 7];

	return buf + 3;
}

static char *prv_dump_ld_ind_off_imm(const specasm_line_t *line, char *buf)
{
	buf = prv_dump_index(line->data.op_code, buf,
			     specasm_line_get_format(line));
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;
	buf += prv_dump_byte(buf, line->data.op_code[3],
			     specasm_line_get_format2(line));

	return buf;
}

static char *prv_dump_ld_special_e(const specasm_line_t *line, char *buf)
{
	char a, b;
	uint8_t opcode1 = line->data.op_code[1];

	if (opcode1 == 0x57) {
		a = 'a';
		b = 'i';
	} else if (opcode1 == 0x47) {
		a = 'i';
		b = 'a';
	} else if (opcode1 == 0x5F) {
		a = 'a';
		b = 'r';
	} else if (opcode1 == 0x4F) {
		a = 'r';
		b = 'a';
	} else {
		goto end;
	}
	buf[0] = a;
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = b;
	buf += 4;

end:
	return buf;
}

static uint8_t prv_dump_ld_sp(uint8_t opcode0, char *buf)
{
	static const char *com[] = {
	    "\xf9"
	    "sp, hl",
	    "\xdd"
	    "sp, ix",
	    "\xfd"
	    "sp, iy",
	};
	return prv_dump_fixed_e(opcode0, buf, com,
				sizeof(com) / sizeof(uint8_t *));
}

static uint8_t prv_dump_ld_e(const specasm_line_t *line, char *buf)
{
	uint8_t i;
	char *start = buf;
	uint8_t opcode0 = line->data.op_code[0];

	/*
	 * Dump immediate variations of the ld instruction
	 * and also ld a, (bc), ld a, (de). ld sp, hl
	 */

	switch (opcode0) {
	case 0xF9:
		buf += prv_dump_ld_sp(opcode0, buf);
		goto end;
	case 0xA:
	case 0x1A:
		buf = prv_dump_lda_ind(opcode0, buf);
		goto end;
	case 0x36:
		buf = prv_dump_ld_16bit_imm_e(line, buf, 6,
					      line->data.op_code[1]);
		goto end;
	case 0x2:
	case 0x12:
		buf = prv_dump_st_reg(opcode0, buf);
		goto end;
	case 0xDD:
	case 0xFD:
		if (line->data.op_code[1] != 0x21)
			break;
		buf = prv_dump_ld_16bit_imm_e(
		    line, buf, opcode0 == 0xDD ? 4 : 5,
		    *((uint16_t *)&line->data.op_code[2]));
		goto end;
	case 0x1:
	case 0x11:
	case 0x21:
	case 0x31:
		buf = prv_dump_ld_16bit_imm_e(
		    line, buf, (opcode0 >> 4) & 3,
		    *((uint16_t *)&line->data.op_code[1]));
		goto end;

	case 0x46:
	case 0x4E:
	case 0x56:
	case 0x5E:
	case 0x66:
	case 0x6E:
	case 0x7E:
		buf = prv_dump_ld_hl_ind_e(opcode0, buf);
		goto end;
	}

	if (opcode0 == 0xED) {
		buf = prv_dump_ld_special_e(line, buf);
		if (buf > start)
			goto end;
	}

	/* Dump reg/imm offset indirect */

	if ((opcode0 == 0xDD) || (opcode0 == 0xFD)) {
		if (line->data.op_code[1] == 0x36) {
			buf = prv_dump_ld_ind_off_imm(line, buf);
			goto end;
		} else if (line->data.op_code[1] == 0xF9) {
			buf += prv_dump_ld_sp(opcode0, buf);
			goto end;
		} else if ((line->data.op_code[1] & 0xC7) == 0x46) {
			buf = prv_dump_ld_reg_off_ind(line, buf);
			goto end;
		} else if ((line->data.op_code[1] & 0xF8) == 0x70) {
			buf = prv_dump_ld_ind_off_reg(line, buf);
			goto end;
		}
	}

	/* Dump immediate indirect variations of the ld instruction */

	switch (opcode0) {
	case 0x22:
	case 0x32:
	case 0xDD:
	case 0xED:
	case 0xFD:
	case 0x2A:
	case 0x3A:
		i = opcode0 > 0x3A ? 1 : 0;
		if ((line->data.op_code[i] & 0x8) == 0x8)
			return prv_dump_ld_ind_imm_rev_e(line, buf);
		else
			return prv_dump_ld_ind_imm_e(line, buf);
	}

	if ((opcode0 & 0xf8) == 0x70) {
		buf = prv_dump_st_reg(opcode0, buf);
		goto end;
	}

	if ((opcode0 & 0xC0) == 0x40) {
		buf = prv_dump_ld_8bit_reg(opcode0, buf);
		goto end;
	}

	if ((opcode0 & 0xC7) == 0x6) {
		i = (opcode0 >> 3) & 7;
		buf[0] = byte_regs[i];
		buf[1] = ',';
		buf[2] = ' ';
		buf += 3;
		buf += prv_dump_byte(buf, line->data.op_code[1],
				     specasm_line_get_format(line));
		return buf - start;
	}

	return 0;

end:
	return buf - start;
}

static uint8_t prv_dump_subtraction_e(const specasm_line_t *line, char *buf,
				      uint8_t id_buf)
{
	char *str;

	str = prv_dump_jump_label_fmt_e(line, buf, line->data.op_code[id_buf],
					line->data.op_code[id_buf + 2]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	str[0] = '-';
	str = prv_dump_jump_label_e(line, str + 1,
				    line->data.op_code[id_buf + 1]);
	return str - buf;
}

static uint8_t prv_dump_ld_imm_16_sub_e(const specasm_line_t *line, char *buf)
{
	const uint8_t *opcode = &line->data.op_code[0];
	char *start = buf;

	buf = prv_dump_16bit_reg(buf, opcode[0]);
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	return prv_dump_subtraction_e(line, buf, 1) + (buf - start);
}

static uint8_t prv_dump_ld_imm_8_sub_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	buf[0] = byte_regs[(line->data.op_code[0] >> 3) & 7];
	buf[1] = ',';
	buf[2] = ' ';
	buf += 3;

	return prv_dump_subtraction_e(line, buf, 1) + (buf - start);
}

static uint8_t prv_dump_or_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xF6, 0xB0) - start;
}

static uint8_t prv_dump_out_e(const specasm_line_t *line, char *buf)
{
	const uint8_t *op_code = line->data.op_code;
	char *start = buf;
	uint8_t code;

	code =
	    (op_code[0] == 0xD3) ? SPECASM_BYTE_REG_A : (op_code[1] >> 3) & 7;

	*buf++ = '(';
	if (op_code[0] == 0xD3) {
		buf += prv_dump_byte(buf, op_code[1],
				     specasm_line_get_format(line));
	} else {
		*buf++ = 'c';
	}
	buf[0] = ')';
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = byte_regs[code];

	return (buf + 4) - start;
}

static uint8_t prv_dump_rst_e(const specasm_line_t *line, char *buf)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t val = op_code[0] & 0x38;

	return prv_dump_byte(buf, val, specasm_line_get_format(line));
}

static uint8_t prv_dump_sbc_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_adc_sbc_e(line, buf, 0xDE, 0x98);
}

static uint8_t prv_dump_stack(const specasm_line_t *line, char *buf)
{
	const uint8_t *op_code = line->data.op_code;
	char a = 'a';
	char b = 'f';

	if (op_code[0] == 0xFD) {
		a = 'i';
		b = 'y';
	} else if (op_code[0] == 0xDD) {
		a = 'i';
		b = 'x';
	} else {
		switch ((op_code[0] >> 4) & 3) {
		case SPECASM_BYTE_REG_BC - SPECASM_BYTE_REG_BC:
			a = 'b';
			b = 'c';
			break;
		case SPECASM_BYTE_REG_DE - SPECASM_BYTE_REG_BC:
			a = 'd';
			b = 'e';
			break;
		case SPECASM_BYTE_REG_HL - SPECASM_BYTE_REG_BC:
			a = 'h';
			b = 'l';
			break;
		default:
			break;
		}
	}

	*buf = a;
	buf[1] = b;

	return 2;
}

static uint8_t prv_dump_ret_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;
	uint8_t op_code = line->data.op_code[0];

	if (op_code != 0xC9) {
		*buf++ = ' ';
		buf = prv_dump_cc(op_code >> 3, buf);
	}

	return buf - start;
}

static uint8_t prv_dump_shift_e(const specasm_line_t *line, char *buf)
{

	char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0xCB)
		buf = prv_byte_and_hl_ind(op_code[1], buf);
	else if (op_code[0] == 0xDD || op_code[0] == 0xFD)
		buf =
		    prv_dump_index(op_code, buf, specasm_line_get_format(line));
	else
		err_type = SPECASM_ERROR_BAD_REG;

	return buf - start;
}

static uint8_t prv_dump_sub_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xD6, 0x90) - start;
}

static uint8_t prv_dump_xor_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xEE, 0xA8) - start;
}

static uint8_t prv_dump_db_e(const specasm_line_t *line, char *buf)
{
	uint8_t i;
	unsigned int len;
	char *ptr;
	uint8_t sz = specasm_line_get_size(line) + 1;

	len = prv_dump_byte(buf, line->data.op_code[0],
			    specasm_line_get_format(line));
	for (i = 1; i < sz; i++) {
		ptr = &buf[len];
		ptr[0] = ',';
		len += prv_dump_byte(ptr + 1, line->data.op_code[i],
				     specasm_line_get_format(line));
		len++;
	}
	return len;
}

static uint8_t prv_dump_equw_e(const specasm_line_t *line, char *buf)
{
	char *str;
	uint16_t id;

	if (specasm_line_get_addr_type(line)) {
		str = prv_dump_jump_label_e(line, buf, line->data.op_code[0]);
	} else {
		id = *((uint16_t *)&line->data.op_code[0]);
		str =
		    buf + prv_dump_word(buf, id, specasm_line_get_format(line));
		if (specasm_line_get_size(line) > 1) {
			str[0] = ',';
			str++;
			id = *((uint16_t *)&line->data.op_code[2]);
			str += prv_dump_word(str, id,
					     specasm_line_get_format(line));
		}
	}
	return str - buf;
}

static uint8_t prv_dump_equws_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_subtraction_e(line, buf, 0);
}

static uint8_t prv_dump_repb_e(const specasm_line_t *line, char *buf)
{
	uint16_t *count;
	char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	buf += prv_dump_byte(buf, op_code[0], specasm_line_get_format(line));
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	count = (uint16_t *)&op_code[1];
	buf += prv_dump_word(buf, *count, specasm_line_get_format2(line));

	return buf - start;
}

static uint8_t prv_dump_org_e(const specasm_line_t *line, char *buf)
{
	uint16_t addr = *((uint16_t *)&line->data.op_code[0]);
	return prv_dump_word(buf, addr, specasm_line_get_format(line));
}

/* clang-format off */

static const specasm_line_opcode_dump_t dump_opcodes[] = {
	{ prv_dump_adc_e, {'a', 'd', 'c' } },       /* SPECASM_LINE_TYPE_ADC */
	{ prv_dump_add_e, {'a', 'd', 'd' } },       /* SPECASM_LINE_TYPE_ADD */
	{ prv_dump_and_e, { 'a', 'n', 'd'} },       /* SPECASM_LINE_TYPE_AND */
	{ prv_dump_bit_e, { 'b', 'i', 't'} },       /* SPECASM_LINE_TYPE_BIT */
	{ prv_dump_call_e, { 'c', 'a', 'l', 'l'} }, /* SPECASM_LINE_TYPE_CALL */
	{ NULL, { 'c', 'c', 'f' } },                /* SPECASM_LINE_TYPE_CCF */
	{ prv_dump_cp_e, {'c', 'p' } },             /* SPECASM_LINE_TYPE_CP */
	{ NULL, { 'c', 'p', 'd' } },                /* SPECASM_LINE_TYPE_CPD */
	{ NULL, { 'c', 'p', 'd', 'r'} },            /* SPECASM_LINE_TYPE_CPDR */
	{ NULL, { 'c', 'p', 'i' } },                /* SPECASM_LINE_TYPE_CPI */
	{ NULL, { 'c', 'p', 'i', 'r' } },           /* SPECASM_LINE_TYPE_CPIR */
	{ NULL, { 'c', 'p', 'l' } },                /* SPECASM_LINE_TYPE_CPL */
	{ NULL, { 'd', 'a', 'a' } },                /* SPECASM_LINE_TYPE_DAA */
	{ prv_dump_dec_e, { 'd', 'e', 'c' } },      /* SPECASM_LINE_TYPE_DEC */
	{ NULL, { 'd', 'i' } },                     /* SPECASM_LINE_TYPE_DI */
	{ prv_dump_djnz_e, { 'd', 'j', 'n', 'z'} }, /* SPECASM_LINE_TYPE_DJNZ */
	{ NULL, { 'e', 'i' } },                     /* SPECASM_LINE_TYPE_EI */
	{ prv_dump_ex_e, { 'e', 'x' } },            /* SPECASM_LINE_TYPE_EX */
	{ NULL, { 'e', 'x', 'x' } },                /* SPECASM_LINE_TYPE_EXX */
	{ NULL, { 'h', 'a', 'l', 't'} },            /* SPECASM_LINE_TYPE_HALT */
	{ prv_dump_im_e, { 'i', 'm'} },             /* SPECASM_LINE_TYPE_IM */
	{ prv_dump_in_e, { 'i', 'n'} },             /* SPECASM_LINE_TYPE_IN */
	{ prv_dump_inc_e, { 'i', 'n', 'c'} },       /* SPECASM_LINE_TYPE_INC */
	{ NULL, { 'i', 'n', 'd'} },                 /* SPECASM_LINE_TYPE_IND */
	{ NULL, { 'i', 'n', 'd', 'r'} },            /* SPECASM_LINE_TYPE_INDR */
	{ NULL, { 'i', 'n', 'i'} },                 /* SPECASM_LINE_TYPE_INI */
	{ NULL, {'i', 'n', 'i', 'r'} },             /* SPECASM_LINE_TYPE_INIR */
	{ prv_dump_jp_e, {'j', 'p'} },              /* SPECASM_LINE_TYPE_JP */
	{ prv_dump_jr_e, {'j', 'r'} },              /* SPECASM_LINE_TYPE_JR */
	{ prv_dump_ld_e, { 'l', 'd'} },             /* SPECASM_LINE_TYPE_LD */
	{ NULL, { 'l', 'd', 'd'} },                 /* SPECASM_LINE_TYPE_LDD */
	{ NULL, { 'l', 'd', 'd', 'r'} },            /* SPECASM_LINE_TYPE_LDDR */
	{ NULL, { 'l', 'd', 'i'} },                 /* SPECASM_LINE_TYPE_LDI */
	{ NULL, { 'l', 'd', 'i', 'r'} },            /* SPECASM_LINE_TYPE_LDIR */
	{ NULL, { 'n', 'e', 'g'} },                 /* SPECASM_LINE_TYPE_NEG */
	{ NULL, { 'n', 'o', 'p'} },                 /* SPECASM_LINE_TYPE_NOP */
	{ prv_dump_or_e, {'o', 'r'} },              /* SPECASM_LINE_TYPE_OR */
	{ NULL, {'o', 't', 'd', 'r'} },             /* SPECASM_LINE_TYPE_OTDR */
	{ NULL, {'o', 't', 'i', 'r'} },             /* SPECASM_LINE_TYPE_OTIR */
	{ prv_dump_out_e, {'o', 'u', 't'} },        /* SPECASM_LINE_TYPE_OUT */
	{ NULL, {'o', 'u', 't', 'd'} },             /* SPECASM_LINE_TYPE_OUTD */
	{ NULL, {'o', 'u', 't', 'i'} },             /* SPECASM_LINE_TYPE_OUTI */
	{ prv_dump_stack, {'p', 'o', 'p'} },        /* SPECASM_LINE_TYPE_POP */
	{ prv_dump_stack, {'p', 'u', 's', 'h'} },   /* SPECASM_LINE_TYPE_PUSH */
	{ prv_dump_bit_e, {'r', 'e', 's'} },        /* SPECASM_LINE_TYPE_RES */
	{ prv_dump_ret_e, {'r', 'e', 't'} },        /* SPECASM_LINE_TYPE_RET */
	{ NULL, {'r', 'e', 't', 'i'} },             /* SPECASM_LINE_TYPE_RETI */
	{ NULL, {'r', 'e', 't', 'n'} },             /* SPECASM_LINE_TYPE_RETN */
	{ prv_dump_shift_e, {'r', 'l' } },          /* SPECASM_LINE_TYPE_RL */
	{ NULL, {'r', 'l', 'a'} },                  /* SPECASM_LINE_TYPE_RLA */
	{ prv_dump_shift_e, {'r', 'l', 'c'} },      /* SPECASM_LINE_TYPE_RLC */
	{ NULL, {'r', 'l', 'c', 'a'} },             /* SPECASM_LINE_TYPE_RLCA */
	{ NULL, {'r', 'l', 'd'} },                  /* SPECASM_LINE_TYPE_RLD */
	{ prv_dump_shift_e, {'r', 'r'} },           /* SPECASM_LINE_TYPE_RR */
	{ NULL, {'r', 'r', 'a'} },                  /* SPECASM_LINE_TYPE_RRA */
	{ prv_dump_shift_e, {'r', 'r', 'c'} },      /* SPECASM_LINE_TYPE_RRC */
	{ NULL, {'r', 'r', 'c', 'a'} },             /* SPECASM_LINE_TYPE_RRCA */
	{ NULL, {'r', 'r', 'd'} },                  /* SPECASM_LINE_TYPE_RRD */
	{ prv_dump_rst_e, {'r', 's', 't'} },        /* SPECASM_LINE_TYPE_RST */
	{ prv_dump_sbc_e, {'s', 'b', 'c'} },        /* SPECASM_LINE_TYPE_SBC */
	{ NULL, {'s', 'c', 'f'} },                  /* SPECASM_LINE_TYPE_SCF */
	{ prv_dump_bit_e, {'s', 'e', 't'} },        /* SPECASM_LINE_TYPE_SET */
	{ prv_dump_shift_e, {'s', 'l', 'a'} },      /* SPECASM_LINE_TYPE_SLA */
	{ prv_dump_shift_e, {'s', 'r', 'a'} },      /* SPECASM_LINE_TYPE_SRA */
	{ prv_dump_shift_e, {'s', 'r', 'l'}},       /* SPECASM_LINE_TYPE_SRL */
	{ prv_dump_sub_e, {'s', 'u', 'b'} },        /* SPECASM_LINE_TYPE_SUB */
	{ prv_dump_xor_e, {'x', 'o', 'r'} },        /* SPECASM_LINE_TYPE_XOR */
	{ prv_dump_db_e, {'d', 'b'} },              /* SPECASM_LINE_TYPE_DB */
	{ prv_dump_equw_e, {'d', 'w'} },            /* SPECASM_LINE_TYPE_DW */
	{ prv_dump_equws_e, { 'd', 'b'} },          /* SPECASM_LINE_TYPE_DB_SUB */
	{ prv_dump_equws_e, { 'd', 'w'} },          /* SPECASM_LINE_TYPE_DW_SUB */
	{ prv_dump_ld_imm_16_sub_e, {'l', 'd' } }, /*SPECASM_LINE_TYPE_LD_IMM_16_SUB */
	{ prv_dump_ld_imm_8_sub_e, {'l', 'd' } }, /*SPECASM_LINE_TYPE_LD_IMM_8_SUB */
	{ prv_dump_repb_e, {'r', 'e', 'p', 'b' } }, /*SPECASM_LINE_TYPE_REPB */
	{ prv_dump_org_e, {'o', 'r', 'g'} },        /* SPECASM_LINE_TYPE_ORG */
	{ NULL, {'m', 'a', 'p'} },                  /* SPECASM_LINE_TYPE_MAP */
};

/* clang-format on */

static uint8_t prv_parse_16bit_unary_e(const char *args, specasm_line_t *line,
				       const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t read;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t sz = 0;
	uint8_t rind = op_entry->op_code[0];
	uint8_t hl_ind = op_entry->op_code[1];

	read = prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	switch (reg) {
	case SPECASM_BYTE_REG_SP:
		reg--;
	case SPECASM_BYTE_REG_BC:
	case SPECASM_BYTE_REG_DE:
	case SPECASM_BYTE_REG_HL:
		line->data.op_code[0] = (reg - SPECASM_BYTE_REG_BC) << 4 | rind;
		break;
	case SPECASM_BYTE_REG_HL_IND:
		line->data.op_code[0] = hl_ind;
		break;
	case SPECASM_BYTE_REG_IX_OFF:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDD;
		op_code[1] = hl_ind;
		op_code[2] = off;
		specasm_line_set_format(line, flags);
		sz = 2;
		break;
	case SPECASM_BYTE_REG_IY_OFF:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xFD;
		op_code[1] = hl_ind;
		op_code[2] = off;
		specasm_line_set_format(line, flags);
		sz = 2;
		break;
	case SPECASM_BYTE_REG_IX:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDD;
		op_code[1] = 0x20 | rind;
		sz = 1;
		break;
	case SPECASM_BYTE_REG_IY:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xFD;
		op_code[1] = 0x20 | rind;
		sz = 1;
		break;
	default:
		if (reg > SPECASM_BYTE_REG_A) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		line->data.op_code[0] = (hl_ind & 0xD) | reg << 3;
		break;
	}

	specasm_line_set_size(line, sz);

	return read;
}

static const char *prv_parse_jump_label_e(const char *args,
					  specasm_line_t *line, uint8_t *label)
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

static const char *prv_get_label_ind_e(const char *args, specasm_line_t *line,
				       uint8_t *val)
{
	while (*args == ' ')
		++args;

	if (*args++ != '(') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	while (*args == ' ')
		++args;

	args = prv_parse_jump_label_e(args, line, val);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	while (*args == ' ')
		++args;

	if (*args != ')') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}

	return args + 1;
}

static uint8_t prv_parse_arith_gen_e(const char *args, specasm_line_t *line,
				     uint8_t areg, uint8_t hl_ind, uint8_t aimm)
{
	uint8_t reg;
	uint8_t off;
	uint8_t read;
	const char *args2;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t sz = 0;

	args2 = prv_get_byte_imm_e(args, &off, &flags);
	if (err_type == SPECASM_ERROR_OK) {
		specasm_line_set_format(line, flags);
		op_code = &line->data.op_code[0];
		op_code[0] = aimm;
		op_code[1] = off;
		read = args2 - args;
		sz = 1;
		goto end;
	}
	if (err_type == SPECASM_ERROR_NUM_TOO_BIG)
		return 0;

	err_type = SPECASM_ERROR_OK;
	read = prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg <= SPECASM_BYTE_REG_A) {
		line->data.op_code[0] = areg | reg;
		goto end;
	}

	if (reg == SPECASM_BYTE_REG_HL_IND) {
		line->data.op_code[0] = hl_ind;
		goto end;
	}

	if (reg == SPECASM_BYTE_REG_IX_OFF) {
		line->data.op_code[0] = 0xDD;
	} else if (reg == SPECASM_BYTE_REG_IY_OFF) {
		line->data.op_code[0] = 0xFD;
	} else {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}
	specasm_line_set_format(line, flags);
	op_code = &line->data.op_code[0];
	op_code[1] = hl_ind;
	op_code[2] = off;
	sz = 2;

end:
	specasm_line_set_size(line, sz);
	return read;
}

static uint8_t prv_parse_adc_sbc_e(const char *args, specasm_line_t *line,
				   const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t aimm;
	uint8_t areg;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t hl_ind;
	const char *start = args;

	args = prv_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (reg == SPECASM_BYTE_REG_A) {
		hl_ind = op_entry->op_code[1] | 6;
		aimm = hl_ind | 0x40;
		areg = op_entry->op_code[1];
		args += prv_parse_arith_gen_e(args, line, areg, hl_ind, aimm);
	} else if (reg == SPECASM_BYTE_REG_HL) {
		args += prv_parse_reg_e(args, &reg, &off, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		op_code = &line->data.op_code[0];
		op_code[0] = 0xED;
		op_code[1] = op_entry->op_code[0];
		switch (reg) {
		case SPECASM_BYTE_REG_BC:
			break;
		case SPECASM_BYTE_REG_DE:
			op_code[1] |= 1 << 4;
			break;
		case SPECASM_BYTE_REG_HL:
			op_code[1] |= 2 << 4;
			break;
		case SPECASM_BYTE_REG_SP:
			op_code[1] |= 3 << 4;
			break;
		default:
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		specasm_line_set_size(line, 1);
	} else {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	return args - start;
}

static uint8_t prv_parse_add_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t reg2;
	uint8_t off;
	uint8_t i;
	uint8_t flags;
	uint8_t opi = 0;
	const char *start = args;
	uint8_t sixbit_regs[] = {SPECASM_BYTE_REG_BC, SPECASM_BYTE_REG_DE,
				 SPECASM_BYTE_REG_HL, SPECASM_BYTE_REG_SP};

	args = prv_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	switch (reg) {
	case SPECASM_BYTE_REG_A:
		args += prv_parse_arith_gen_e(args, line, 0x80, 0x86, 0xC6);
		return args - start;
	case SPECASM_BYTE_REG_HL:
		break;
	case SPECASM_BYTE_REG_IY:
		sixbit_regs[2] = SPECASM_BYTE_REG_IY;
		line->data.op_code[0] = 0xFD;
		opi++;
		break;
	case SPECASM_BYTE_REG_IX:
		sixbit_regs[2] = SPECASM_BYTE_REG_IX;
		line->data.op_code[0] = 0xDD;
		opi++;
		break;
	default:
		goto bad_reg;
	}
	args += prv_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	for (i = 0; i < 4; i++)
		if (sixbit_regs[i] == reg2) {
			line->data.op_code[opi] = 0x9 | (i << 4);
			specasm_line_set_size(line, opi);
			return args - start;
		}

bad_reg:
	err_type = SPECASM_ERROR_BAD_REG;
	return 0;
}

static uint8_t prv_parse_arith_e(const char *args, specasm_line_t *line,
				 const specasm_opcode_t *op_entry)
{
	uint8_t aimm = op_entry->op_code[0];
	uint8_t areg = op_entry->op_code[1];

	return prv_parse_arith_gen_e(args, line, areg, areg | 6, aimm);
}

static uint8_t prv_parse_org_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t flags;
	uint16_t val;
	const char *start = args;

	args = prv_get_uword_imm_e(args, &val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	*((uint16_t *)&line->data.op_code[0]) = val;
	specasm_line_set_format(line, flags);

	return args - start;
}

static uint8_t prv_parse_bit_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t val;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t sz = 1;
	const char *start = args;
	uint8_t rind = op_entry->op_code[0];
	uint8_t hl_ind = 0xCB;

	args = prv_get_byte_imm_e(args, &val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (val > 7) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	while (*args == ' ')
		++args;

	if (*args != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	args++;
	args += prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	switch (reg) {
	case SPECASM_BYTE_REG_HL_IND:
		op_code = &line->data.op_code[0];
		op_code[0] = hl_ind;
		op_code[1] = rind | (val << 3) | 0x6;
		break;
	case SPECASM_BYTE_REG_IX_OFF:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDD;
		op_code[1] = hl_ind;
		op_code[2] = off;
		op_code[3] = rind | (val << 3) | 0x6;
		specasm_line_set_format(line, flags);
		sz = 3;
		break;
	case SPECASM_BYTE_REG_IY_OFF:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xFD;
		op_code[1] = hl_ind;
		op_code[2] = off;
		op_code[3] = rind | (val << 3) | 0x6;
		specasm_line_set_format(line, flags);
		sz = 3;
		break;
	default:
		if (reg > SPECASM_BYTE_REG_A) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = hl_ind;
		op_code[1] = rind | (val << 3) | reg;
		break;
	}

	specasm_line_set_size(line, sz);
	return args - start;
}

static const char *prv_parse_cc_e(const char *args, uint8_t *code)
{
	while (*args == ' ')
		++args;

	if (*args == ';' || !*args) {
		*code = SPECASM_CC_NONE;
		return args;
	}

	if (*args == 'p') {
		if (args[1] == 'o') {
			*code = SPECASM_CC_PO;
			args++;
		} else if (args[1] == 'e') {
			*code = SPECASM_CC_PE;
			args++;
		} else {
			*code = SPECASM_CC_P;
		}
	} else if (*args == 'n') {
		args++;
		if (*args == 'z')
			*code = SPECASM_CC_NZ;
		else if (*args == 'c')
			*code = SPECASM_CC_NC;
		else
			goto on_error;
	} else {
		switch (*args) {
		case 'z':
			*code = SPECASM_CC_Z;
			break;
		case 'c':
			*code = SPECASM_CC_C;
			break;
		case 'm':
			*code = SPECASM_CC_M;
			break;
		default:
			goto on_error;
		}
	}
	++args;

	if (*args != 0 && *args != ' ' && *args != ',')
		goto on_error;

	return args;

on_error:

	err_type = SPECASM_ERROR_CONDITION_CODE;
	return 0;
}

static uint8_t prv_parse_djnz_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	const char *start = args;

	while (*args == ' ')
		++args;

	args = prv_parse_jump_label_e(args, line, &line->data.op_code[1]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	line->data.op_code[0] = 0x10;
	specasm_line_set_size(line, 1);

	return args - start;
}

static const char *prv_parse_labels_e(const char *args, specasm_line_t *line,
				      uint8_t *count, uint16_t *val,
				      uint8_t *addr_fmt2)
{
	const char *args2;
	uint8_t *labels = (uint8_t *)val;

	/*
	 * So this is all a bit hacky.  There aren't enough bits in
	 * flags to store the address type for two addresses.  There's
	 * only room for one.  However, all the instructions that allow
	 * address subtraction have at least one spare byte in the opcode.
	 * So, we'll store the flags for the second label in the address
	 * bits, and the flags for the first label in the first unused
	 * byte in the opcode.  If we ever add a second flags byte we
	 * can clean this up.
	 */

	*count = 1;
	while (*args == ' ')
		++args;
	args = prv_parse_jump_label_e(args, line, &labels[0]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	args2 = args;
	while (*args2 == ' ')
		++args2;
	*addr_fmt2 = 0;
	if (*args2 == '-') {
		*addr_fmt2 = specasm_line_get_addr_type(line);

		/* Again this is hacky.  specasm_line_set_addr_type assumes
		 * that the address bits are zero.  If they're not it doesn't
		 * work correctly.  We need to explicitly reset them.
		 */

		line->flags &= 0xF3;
		*count = 2;
		args2++;
		while (*args2 == ' ')
			++args2;
		args = prv_parse_jump_label_e(args2, line, &labels[1]);
	}

	return args;
}

static uint8_t prv_signed_ok(uint8_t flags, uint8_t val)
{
	return (flags == SPECASM_FLAGS_NUM_SIGNED) ||
	       ((flags == SPECASM_FLAGS_NUM_UNSIGNED) && (val < 128));
}

static uint8_t prv_parse_db_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	uint8_t val;
	uint8_t flags;
	uint8_t flags2;
	uint8_t signed_ok;
	uint8_t label_count;
	uint8_t *op_code;
	const char *args2;
	uint8_t i = 0;
	const char *start = args;

	args = prv_get_byte_imm_e(args, &val, &flags);
	if (err_type == SPECASM_ERROR_NUM_TOO_BIG)
		return 0;
	if (err_type != SPECASM_ERROR_OK) {
		args2 = start;
		err_type = SPECASM_ERROR_OK;
		op_code = &line->data.op_code[0];
		args = prv_parse_labels_e(args2, line, &label_count,
					  (uint16_t *)op_code, &op_code[2]);

		if (err_type != SPECASM_ERROR_OK)
			return 0;
		if (label_count != 2) {
			err_type = SPECASM_ERROR_BAD_LABEL;
			return 0;
		}

		line->type = SPECASM_LINE_TYPE_DB_SUB;
		return args - start;
	}
	line->data.op_code[0] = val;

	for (i = 1; i < 4; i++) {
		args2 = args;
		while (*args2 == ' ')
			++args2;
		if (*args2 != ',')
			break;
		signed_ok = prv_signed_ok(flags, val);
		args = prv_get_byte_imm_e(args2 + 1, &val, &flags2);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		if (flags2 != flags) {
			if (signed_ok)
				signed_ok = prv_signed_ok(flags2, val);
			if (!signed_ok) {
				err_type = SPECASM_ERROR_BAD_NUM;
				return 0;
			}
			flags = SPECASM_FLAGS_NUM_SIGNED;
		}
		line->data.op_code[i] = val;
	}
	specasm_line_set_format(line, flags);
	specasm_line_set_size(line, i - 1);

	return args - start;
}

static uint8_t prv_signed_ok_u16(uint8_t flags, uint16_t val)
{
	return (flags == SPECASM_FLAGS_NUM_SIGNED) ||
	       ((flags == SPECASM_FLAGS_NUM_UNSIGNED) && (val < 32768));
}

static uint8_t prv_parse_dw_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	uint16_t val;
	uint8_t flags;
	uint8_t flags2;
	uint8_t signed_ok;
	uint8_t label_count;
	uint8_t *op_code;
	const char *args2 = args;
	int8_t sz = 1;
	const char *start = args;

	args = prv_get_word_imm_e(args, &val, &flags);
	if (err_type == SPECASM_ERROR_NUM_TOO_BIG)
		return 0;
	if (err_type == SPECASM_ERROR_OK) {
		*((uint16_t *)&line->data.op_code[0]) = val;
		args2 = args;
		while (*args2 == ' ')
			++args2;
		if (*args2 == ',') {
			signed_ok = prv_signed_ok_u16(flags, val);
			args = prv_get_word_imm_e(args2 + 1, &val, &flags2);
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			if (flags2 != flags) {
				if (signed_ok)
					signed_ok =
					    prv_signed_ok_u16(flags2, val);
				if (!signed_ok) {
					err_type = SPECASM_ERROR_BAD_NUM;
					return 0;
				}
				flags = SPECASM_FLAGS_NUM_SIGNED;
			}
			*((uint16_t *)&line->data.op_code[2]) = val;
			sz = 3;
		}
		specasm_line_set_format(line, flags);
	} else {
		err_type = SPECASM_ERROR_OK;
		op_code = &line->data.op_code[0];
		args = prv_parse_labels_e(args2, line, &label_count,
					  (uint16_t *)op_code, &op_code[2]);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		if (label_count == 2)
			line->type = SPECASM_LINE_TYPE_DW_SUB;
	}

	specasm_line_set_size(line, sz);

	return args - start;
}

static uint8_t prv_parse_ex_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	uint8_t reg1;
	uint8_t off1;
	uint8_t reg2;
	uint8_t off2;
	uint8_t flags;
	uint8_t *op_code;
	const char *args2;
	uint8_t sz = 0;

	args2 = prv_parse_reg_comma_e(args, &reg1, &off1, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	args2 += prv_parse_reg_e(args2, &reg2, &off2, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	op_code = &line->data.op_code[0];
	if ((reg1 == SPECASM_BYTE_REG_AF) && (reg2 == SPECASM_BYTE_REG_AF_P))
		op_code[0] = 0x8;
	else if ((reg1 == SPECASM_BYTE_REG_DE) && (reg2 == SPECASM_BYTE_REG_HL))
		op_code[0] = 0xEB;
	else if (reg1 == SPECASM_BYTE_REG_SP_IND) {
		if (reg2 == SPECASM_BYTE_REG_HL) {
			op_code[0] = 0xE3;
		} else if (reg2 == SPECASM_BYTE_REG_IX) {
			op_code[0] = 0xDD;
			op_code[1] = 0xE3;
			sz++;
		} else if (reg2 == SPECASM_BYTE_REG_IY) {
			op_code[0] = 0xFD;
			op_code[1] = 0xE3;
			sz++;
		} else {
			goto bad_reg;
		}
	} else {
		goto bad_reg;
	}

	specasm_line_set_size(line, sz);

	return args2 - args;

bad_reg:
	err_type = SPECASM_ERROR_BAD_REG;
	return 0;
}

static uint8_t prv_parse_im_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	const char *args2;
	uint8_t val;
	uint8_t flags;

	args2 = prv_get_byte_imm_e(args, &val, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	specasm_line_set_format(line, flags);

	switch (val) {
	case 0:
		line->data.op_code[1] = 0x46;
		break;
	case 1:
		line->data.op_code[1] = 0x56;
		break;
	case 2:
		line->data.op_code[1] = 0x5E;
		break;
	default:
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	line->data.op_code[0] = 0xED;
	specasm_line_set_size(line, 1);

	return args2 - args;
}

static uint8_t prv_parse_in_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	const char *args2;
	uint8_t val;
	uint8_t reg;
	uint8_t reg2;
	uint8_t off;
	uint8_t flags;
	uint8_t *op_code;
	const char *start = args;

	args = prv_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (reg > SPECASM_BYTE_REG_A) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	args2 = args + prv_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		if (reg != SPECASM_BYTE_REG_A)
			return 0;
		err_type = SPECASM_ERROR_OK;

		args = prv_get_byte_imm_ind_e(args, &val, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		specasm_line_set_format(line, flags);
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDB;
		op_code[1] = val;
	} else {
		if (reg2 != SPECASM_BYTE_REG_C_IND) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = 0xED;
		op_code[1] = 0x40 | (reg << 3);
		args = args2;
	}

	specasm_line_set_size(line, 1);

	return args - start;
}

static uint8_t prv_parse_jp_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	uint8_t cc;
	const char *args2;
	uint8_t read;
	uint8_t off;
	uint8_t reg;
	uint8_t condition;
	uint8_t sz;
	uint8_t flags;
	uint8_t op1;
	uint8_t op2 = 0;
	uint8_t *op_code;
	const char *start = args;

	read = prv_parse_reg_e(args, &reg, &off, &flags);
	if ((err_type == SPECASM_ERROR_OK) && (reg != SPECASM_BYTE_REG_C)) {
		sz = 1;
		switch (reg) {
		case SPECASM_BYTE_REG_HL_IND:
			op1 = 0xE9;
			sz = 0;
			break;
		case SPECASM_BYTE_REG_IX_IND:
			op1 = 0xDD;
			op2 = 0xE9;
			break;
		case SPECASM_BYTE_REG_IY_IND:
			op1 = 0xFD;
			op2 = 0xE9;
			break;
		default:
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = op1;
		op_code[1] = op2;
		specasm_line_set_size(line, sz);
		return read;
	}

	err_type = SPECASM_ERROR_OK;
	args2 = prv_parse_cc_e(args, &cc);
	condition = err_type == SPECASM_ERROR_OK && cc != SPECASM_CC_NONE;
	if (condition) {
		if (*args2 != ' ' && *args2 != ',') {
			err_type = SPECASM_ERROR_CONDITION_CODE;
			return 0;
		}
		while (*args2 == ' ')
			++args2;
		if (*args2 != ',') {
			err_type = SPECASM_ERROR_COMMA_EXPECTED;
			return 0;
		}
		args = args2 + 1;
	} else {
		err_type = SPECASM_ERROR_OK;
	}

	while (*args == ' ')
		++args;

	args = prv_parse_jump_label_e(args, line, &line->data.op_code[1]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (condition) {
		line->data.op_code[0] = 0xC2 | (cc << 3);
	} else {
		args2 = args;
		while (*args2 == ' ')
			++args2;
		if (*args2 == ',') {
			err_type = SPECASM_ERROR_CONDITION_CODE;
			return 0;
		}
		line->data.op_code[0] = 0xC3;
	}

	specasm_line_set_size(line, 2);

	return args - start;
}

static uint8_t prv_parse_relative_jmp_e(const char *args, specasm_line_t *line,
					const specasm_opcode_t *op_entry,
					uint8_t max_cc)
{
	const char *args2;
	uint8_t label;
	uint8_t cc;
	uint8_t cc_ind = op_entry->op_code[0];
	uint8_t abs_ind = op_entry->op_code[1];
	const char *start = args;

	args2 = prv_parse_cc_e(args, &cc);
	if (err_type == SPECASM_ERROR_OK && cc != SPECASM_CC_NONE) {
		if ((*args2 != ' ' && *args2 != ',') || (cc > max_cc)) {
			err_type = SPECASM_ERROR_CONDITION_CODE;
			return 0;
		}
		line->data.op_code[0] = cc_ind | cc << 3;
		;
		while (*args2 == ' ')
			++args2;
		if (*args2 != ',') {
			err_type = SPECASM_ERROR_COMMA_EXPECTED;
			return 0;
		}
		args = args2 + 1;
	} else {
		line->data.op_code[0] = abs_ind;
		err_type = SPECASM_ERROR_OK;
	}

	while (*args == ' ')
		++args;

	args2 = prv_parse_jump_label_e(args, line, &label);

	/* If the labels bad we might have a 16 bit offset so return
	 * an error but skip past the condition code if any.
	 */

	if (err_type != SPECASM_ERROR_OK)
		return args - start;

	if (line->data.op_code[0] == abs_ind) {
		args = args2;
		while (*args == ' ')
			++args;
		if (*args == ',') {
			err_type = SPECASM_ERROR_CONDITION_CODE;
			return 0;
		}
	}

	line->data.op_code[1] = label;

	return args2 - start;
}

static uint8_t prv_parse_absolute_jmp_e(const char *args, specasm_line_t *line,
					const specasm_opcode_t *op_entry,
					uint8_t max_cc)
{
	uint8_t len;
	const char *start;
	uint16_t val;
	uint8_t flags;

	len = prv_parse_relative_jmp_e(args, line, op_entry, max_cc);
	if (err_type != SPECASM_ERROR_BAD_LABEL)
		return len;

	start = args;
	err_type = SPECASM_ERROR_OK;
	args = prv_get_uword_imm_e(args + len, &val, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SPECASM_ERROR_BAD_LABEL;
		return 0;
	}
	*((uint16_t *)&line->data.op_code[1]) = val;
	specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
	specasm_line_set_format(line, flags);

	return args - start;
}

static uint8_t prv_parse_call_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	specasm_line_set_size(line, 2);
	return prv_parse_absolute_jmp_e(args, line, op_entry, SPECASM_CC_M);
}

static uint8_t prv_parse_jr_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	specasm_line_set_size(line, 1);
	return prv_parse_relative_jmp_e(args, line, op_entry, SPECASM_CC_C);
}

static void prv_set_ld_imm_ind_e(specasm_line_t *line, uint16_t val,
				 uint8_t reg, uint8_t mod)
{

	uint8_t opc1;
	uint8_t opc2 = 0;
	uint8_t lab_loc = 2;
	uint8_t *op_code;

	switch (reg) {
	case SPECASM_BYTE_REG_A:
		opc1 = 0x32;
		lab_loc = 1;
		break;
	case SPECASM_BYTE_REG_BC:
		opc1 = 0xED;
		opc2 = 0x43;
		break;
	case SPECASM_BYTE_REG_DE:
		opc1 = 0xED;
		opc2 = 0x53;
		break;
	case SPECASM_BYTE_REG_HL:
		opc1 = 0x22;
		lab_loc = 1;
		break;
	case SPECASM_BYTE_REG_SP:
		opc1 = 0xED;
		opc2 = 0x73;
		break;
	case SPECASM_BYTE_REG_IX:
		opc1 = 0xDD;
		opc2 = 0x22;
		break;
	case SPECASM_BYTE_REG_IY:
		opc1 = 0xFD;
		opc2 = 0x22;
		break;
	default:
		err_type = SPECASM_ERROR_BAD_REG;
		return;
	}

	op_code = &line->data.op_code[0];
	op_code[0] = opc1;
	op_code[1] = opc2;
	op_code[lab_loc - 1] |= mod;

	*((uint16_t *)&op_code[lab_loc]) = val;
	specasm_line_set_size(line, lab_loc + 1);
}

static const char *prv_parse_st_ind_imm_e(const char *args,
					  specasm_line_t *line, uint16_t val)
{
	uint8_t reg;
	uint8_t off;
	uint8_t flags;

	while (*args == ' ')
		++args;

	if (*args++ != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	args += prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_set_ld_imm_ind_e(line, val, reg, 0);

	return args;
}

static void prv_ld_16bit_imm_e(specasm_line_t *line, uint8_t reg, uint8_t flags,
			       uint16_t val)
{
	uint8_t *op_code;
	uint8_t rind;
	uint8_t i = 0;

	op_code = &line->data.op_code[0];
	switch (reg) {
	case SPECASM_BYTE_REG_BC:
		rind = 0x1;
		break;
	case SPECASM_BYTE_REG_DE:
		rind = 0x11;
		break;
	case SPECASM_BYTE_REG_HL:
		rind = 0x21;
		break;
	case SPECASM_BYTE_REG_SP:
		rind = 0x31;
		break;
	case SPECASM_BYTE_REG_IX:
		op_code[0] = 0xDD;
		rind = 0x21;
		i++;
		break;
	case SPECASM_BYTE_REG_IY:
		op_code[0] = 0xFD;
		rind = 0x21;
		i++;
		break;
	default:
		err_type = SPECASM_ERROR_BAD_REG;
		return;
	}

	op_code[i++] = rind;
	*((uint16_t *)&op_code[i]) = val;
	specasm_line_set_size(line, i + 1);
	specasm_line_set_format(line, flags);
}

static void prv_ld_imm_e(specasm_line_t *line, uint8_t reg, uint8_t off,
			 uint8_t flags, uint8_t flags2, uint16_t val)
{
	uint8_t sz = 1;
	int16_t sval;
	uint8_t *op_code;

	if (reg <= SPECASM_BYTE_REG_A) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0x6 | (reg << 3);
		op_code[1] = (uint8_t)val;
		goto set_format;
	}

	if (reg == SPECASM_BYTE_REG_HL_IND) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0x36;
		op_code[1] = (uint8_t)val;
		goto set_format;
	}

	if ((reg == SPECASM_BYTE_REG_IX_OFF) ||
	    (reg == SPECASM_BYTE_REG_IY_OFF)) {
		op_code = &line->data.op_code[0];
		op_code[0] = (reg == SPECASM_BYTE_REG_IX_OFF) ? 0xDD : 0xFD;
		op_code[1] = 0x36;
		op_code[2] = off;
		op_code[3] = (uint8_t)val;
		sz = 3;
		specasm_line_set_format(line, flags);
		specasm_line_set_format2(line, flags2);
		goto check_byte;
	}

	prv_ld_16bit_imm_e(line, reg, flags2, val);

	return;

set_format:
	specasm_line_set_format(line, flags2);

check_byte:
	sval = (int16_t)val;
	if ((sval > 255) || (sval < -128)) {
		err_type = SPECASM_ERROR_NUM_TOO_BIG;
		return;
	}

	specasm_line_set_size(line, sz);
}

static void prv_store_ind_reg_e(specasm_line_t *line, uint8_t reg, uint8_t reg2)
{
	uint8_t *op_code = &line->data.op_code[0];

	/*
	 * Size is implicitly set to 0.
	 */

	if (reg == SPECASM_BYTE_REG_HL_IND) {
		if (reg2 > SPECASM_BYTE_REG_A)
			goto fail;
		op_code[0] = 0x70 | reg2;
		return;
	}

	if (reg2 != SPECASM_BYTE_REG_A)
		goto fail;

	if (reg == SPECASM_BYTE_REG_BC_IND)
		op_code[0] = 0x2;
	else if (reg == SPECASM_BYTE_REG_DE_IND)
		op_code[0] = 0x12;
	else
		goto fail;

	return;

fail:
	err_type = SPECASM_ERROR_BAD_REG;
}

static void prv_ld_reg_reg_e(specasm_line_t *line, uint8_t reg, uint8_t off,
			     uint8_t reg2, uint8_t off2, uint8_t flags)
{
	uint8_t *op_code;
	uint8_t sz = 0;
	uint8_t hl_ind[] = {0x46, 0x4E, 0x56, 0x5E, 0x66, 0x6E, 0, 0x7E};

	if (reg & SPECASM_IND_MOD) {
		prv_store_ind_reg_e(line, reg, reg2);
		return;
	} else if (reg <= SPECASM_BYTE_REG_A) {
		if (reg2 <= SPECASM_BYTE_REG_A) {
			op_code = &line->data.op_code[0];
			op_code[0] = 0x40 | reg2 | (reg << 3);
		} else if ((reg2 == SPECASM_BYTE_REG_IX_OFF) ||
			   (reg2 == SPECASM_BYTE_REG_IY_OFF)) {
			op_code = &line->data.op_code[0];
			op_code[0] =
			    (reg2 == SPECASM_BYTE_REG_IX_OFF) ? 0xdd : 0xfd;
			op_code[1] = 0x46 | (reg << 3);
			op_code[2] = off2;
			specasm_line_set_format(line, flags);
			sz = 2;
		} else if (reg2 == SPECASM_BYTE_REG_HL_IND) {
			op_code = &line->data.op_code[0];
			op_code[0] = hl_ind[reg];
		} else if (reg == SPECASM_BYTE_REG_A) {
			switch (reg2) {
			case SPECASM_BYTE_REG_BC_IND:
				op_code = &line->data.op_code[0];
				op_code[0] = 0xA;
				break;
			case SPECASM_BYTE_REG_DE_IND:
				op_code = &line->data.op_code[0];
				op_code[0] = 0x1A;
				break;
			case SPECASM_BYTE_REG_I:
				op_code = &line->data.op_code[0];
				op_code[0] = 0xED;
				op_code[1] = 0x57;
				sz = 1;
				break;
			case SPECASM_BYTE_REG_R:
				op_code = &line->data.op_code[0];
				op_code[0] = 0xED;
				op_code[1] = 0x5F;
				sz = 1;
				break;
			default:
				goto fail;
			}
		} else {
			goto fail;
		}
	} else if ((reg == SPECASM_BYTE_REG_I) || (reg == SPECASM_BYTE_REG_R)) {
		if (reg2 != SPECASM_BYTE_REG_A)
			goto fail;
		op_code = &line->data.op_code[0];
		op_code[0] = 0xED;
		op_code[1] = reg == SPECASM_BYTE_REG_I ? 0x47 : 0x4F;
		sz = 1;
	} else if (reg == SPECASM_BYTE_REG_SP) {
		switch (reg2) {
		case SPECASM_BYTE_REG_HL:
			op_code = &line->data.op_code[0];
			op_code[0] = 0xF9;
			break;
		case SPECASM_BYTE_REG_IX:
			op_code = &line->data.op_code[0];
			op_code[0] = 0xDD;
			op_code[1] = 0xF9;
			sz = 1;
			break;
		case SPECASM_BYTE_REG_IY:
			op_code = &line->data.op_code[0];
			op_code[0] = 0xFD;
			op_code[1] = 0xF9;
			sz = 1;
			break;
		default:
			goto fail;
		}
	} else if (reg2 <= SPECASM_BYTE_REG_A) {
		if ((reg == SPECASM_BYTE_REG_IX_OFF) ||
		    (reg == SPECASM_BYTE_REG_IY_OFF)) {
			op_code = &line->data.op_code[0];
			op_code[0] =
			    (reg == SPECASM_BYTE_REG_IX_OFF) ? 0xdd : 0xfd;
			op_code[1] = 0x70 | reg2;
			op_code[2] = off;
			specasm_line_set_format(line, flags);
			sz = 2;
		} else {
			goto fail;
		}
	} else {
		goto fail;
	}

	specasm_line_set_size(line, sz);

	return;

fail:
	err_type = SPECASM_ERROR_BAD_REG;
}

static void prv_parse_ld_labels_e(specasm_line_t *line, uint8_t reg,
				  uint16_t val, uint8_t label_count,
				  uint8_t addr_fmt2)
{
	uint8_t flags2;
	uint8_t *op_code;

	flags2 = specasm_line_get_format(line);
	if (reg <= SPECASM_BYTE_REG_A) {
		if (label_count != 2) {
			err_type = SPECASM_ERROR_BAD_LABEL;
			return;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = 0x6 | (reg << 3);
		*((uint16_t *)&op_code[1]) = val;
		specasm_line_set_format(line, flags2);
		specasm_line_set_size(line, 1);
		line->type = SPECASM_LINE_TYPE_LD_IMM_8_SUB;
	} else {
		prv_ld_16bit_imm_e(line, reg, flags2, val);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (label_count > 1) {
			if (specasm_line_get_size(line) == 3) {
				err_type = SPECASM_ERROR_BAD_LABEL;
				return;
			}
			line->type = SPECASM_LINE_TYPE_LD_IMM_16_SUB;
		}
	}
	line->data.op_code[3] = addr_fmt2;
}

static uint8_t prv_parse_ld_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	const char *args2;
	uint16_t val;
	uint8_t label;
	uint8_t reg;
	uint8_t off;
	uint8_t reg2;
	uint8_t off2;
	uint8_t flags;
	uint8_t flags2;
	uint8_t label_count;
	uint8_t addr_fmt2;
	const char *start = args;

	args2 = prv_get_word_imm_ind_e(args, &val, &flags);
	if (err_type == SPECASM_ERROR_OK) {
		specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
		specasm_line_set_format(line, flags);
	} else if ((err_type == SPECASM_ERROR_NUM_TOO_BIG) ||
		   (err_type == SPECASM_ERROR_NUM_NEG)) {
		return 0;
	} else {
		err_type = SPECASM_ERROR_OK;
		args2 = prv_get_label_ind_e(args, line, &label);
		val = label;
	}
	if (err_type == SPECASM_ERROR_OK) {
		args = prv_parse_st_ind_imm_e(args2, line, val);
		return args - start;
	}

	/*
	  For some reason using prv_parse_reg_comma_e here actually makes
	  the program bigger.
	*/

	err_type = SPECASM_ERROR_OK;
	args += prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	while (*args == ' ')
		++args;

	if (*args++ != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}
	args2 = prv_get_word_imm_ind_e(args, &val, &flags);
	if ((err_type == SPECASM_ERROR_NUM_TOO_BIG) ||
	    (err_type == SPECASM_ERROR_NUM_NEG)) {
		return 0;
	} else if (err_type == SPECASM_ERROR_OK) {
		specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
		specasm_line_set_format(line, flags);
	} else {
		err_type = SPECASM_ERROR_OK;
		args2 = prv_get_label_ind_e(args, line, &label);
		val = label;
	}
	if (err_type == SPECASM_ERROR_OK) {
		prv_set_ld_imm_ind_e(line, val, reg, 0x8);
		return args2 - start;
	}
	err_type = SPECASM_ERROR_OK;
	args2 = prv_get_word_imm_e(args, &val, &flags2);
	if (err_type == SPECASM_ERROR_NUM_TOO_BIG) {
		return 0;
	} else if (err_type == SPECASM_ERROR_OK) {
		prv_ld_imm_e(line, reg, off, flags, flags2, val);
		return args2 - start;
	} else if ((reg <= SPECASM_BYTE_REG_A) ||
		   (reg >= SPECASM_BYTE_REG_BC && reg <= SPECASM_BYTE_REG_IY)) {
		/*
		 * For 16 bit immediate loads we allow labels and label
		 * subtraction in place of the 16 bit value.  For 8 bit
		 * immediate loads we allow label subtraction.
		 */

		err_type = SPECASM_ERROR_OK;
		args2 = prv_parse_labels_e(args, line, &label_count, &val,
					   &addr_fmt2);
		if (err_type == SPECASM_ERROR_OK) {
			prv_parse_ld_labels_e(line, reg, val, label_count,
					      addr_fmt2);
			return args2 - start;
		}
	}

	err_type = SPECASM_ERROR_OK;
	args += prv_parse_reg_e(args, &reg2, &off2, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_ld_reg_reg_e(line, reg, off, reg2, off2, flags);

	return args - start;
}

static uint8_t prv_parse_out_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	const char *args2;
	uint8_t reg;
	uint8_t reg2;
	uint8_t off;
	uint8_t n;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t val = 0;
	const char *start = args;

	args2 = args + prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SPECASM_ERROR_OK;
		args = prv_get_byte_imm_ind_e(args, &val, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		specasm_line_set_format(line, flags);
		n = 1;
	} else {
		args = args2;
		n = 0;
	}

	while (*args == ' ')
		++args;

	if (*args != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}
	++args;

	args += prv_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (n) {
		if (reg2 != SPECASM_BYTE_REG_A) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = 0xD3;
		op_code[1] = val;
	} else {
		if ((reg != SPECASM_BYTE_REG_C_IND) ||
		    (reg2 > SPECASM_BYTE_REG_A)) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		op_code = &line->data.op_code[0];
		op_code[0] = 0xED;
		op_code[1] = 0x41 | (reg2 << 3);
	}

	specasm_line_set_size(line, 1);
	return args - start;
}

static uint8_t prv_parse_push_pop_e(const char *args, specasm_line_t *line,
				    const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t read;
	uint8_t flags;
	uint8_t sz = 1;
	uint8_t *op_code;
	uint8_t ainstr = op_entry->op_code[0];

	read = prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	switch (reg) {
	case SPECASM_BYTE_REG_BC:
	case SPECASM_BYTE_REG_HL:
	case SPECASM_BYTE_REG_DE:
	case SPECASM_BYTE_REG_AF:
		op_code = &line->data.op_code[0];
		op_code[0] = ((reg - SPECASM_BYTE_REG_BC) << 4) | ainstr | 0xC0;
		sz = 0;
		break;
	case SPECASM_BYTE_REG_IX:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDD;
		op_code[1] = 0xE0 | ainstr;
		break;
	case SPECASM_BYTE_REG_IY:
		op_code = &line->data.op_code[0];
		op_code[0] = 0xFD;
		op_code[1] = 0xE0 | ainstr;
		break;
	default:
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	specasm_line_set_size(line, sz);

	return read;
}

static uint8_t prv_parse_rst_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t val;
	uint8_t flags;
	const char *start = args;

	args = prv_get_byte_imm_e(args, &val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if ((val > 0x38) || (val & 7)) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}
	specasm_line_set_format(line, flags);

	line->data.op_code[0] = 0xC7 | val;

	return args - start;
}

static uint8_t prv_parse_repb_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	uint16_t *count;
	uint8_t flags;
	const char *args2;
	uint8_t *op_code = &line->data.op_code[0];

	args2 = prv_get_byte_imm_e(args, op_code, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	specasm_line_set_format(line, flags);
	while (*args2 == ' ')
		++args2;

	if (*args2 != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	count = (uint16_t *)(&op_code[1]);
	args2 = prv_get_uword_imm_e(args2 + 1, count, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (*count == 0) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	specasm_line_set_format2(line, flags);

	return args2 - args;
}

static uint8_t prv_parse_ret_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t cc;
	const char *args2;

	args2 = prv_parse_cc_e(args, &cc);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (cc == SPECASM_CC_NONE) {
		err_type = SPECASM_ERROR_OK;
		line->data.op_code[0] = 0xC9;
		return 0;
	}

	if (*args2 != ' ' && *args2 != ';' && *args2 != '\0') {
		err_type = SPECASM_ERROR_CONDITION_CODE;
		return 0;
	}
	line->data.op_code[0] = 0xC0 | (cc << 3);

	return args2 - args;
}

static uint8_t prv_parse_shift_e(const char *args, specasm_line_t *line,
				 const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t read;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t sz = 1;
	uint8_t hl_ind = op_entry->op_code[0];

	read = prv_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (reg <= SPECASM_BYTE_REG_A) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0xCB;
		op_code[1] = (hl_ind & 0x38) | reg;
		goto end;
	}

	if (reg == SPECASM_BYTE_REG_HL_IND) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0xCB;
		op_code[1] = hl_ind;
		goto end;
	}

	if (reg == SPECASM_BYTE_REG_IX_OFF) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0xDD;
		specasm_line_set_format(line, flags);
	} else if (reg == SPECASM_BYTE_REG_IY_OFF) {
		op_code = &line->data.op_code[0];
		op_code[0] = 0xFD;
		specasm_line_set_format(line, flags);
	} else {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}
	op_code = &line->data.op_code[0];
	op_code[1] = 0xCB;
	op_code[2] = off;
	op_code[3] = hl_ind;
	sz = 3;

end:
	specasm_line_set_size(line, sz);
	return read;
}

/* clang-format off */

const static specasm_opcode_t opcode_table[] = {
	{ "adc", prv_parse_adc_sbc_e, SPECASM_LINE_TYPE_ADC, {0x4A, 0x88} },
	{ "add", prv_parse_add_e, SPECASM_LINE_TYPE_ADD, },
	{ "and", prv_parse_arith_e, SPECASM_LINE_TYPE_AND, {0xE6, 0xA0} },
	{ "bit", prv_parse_bit_e, SPECASM_LINE_TYPE_BIT, { 0x40 } },
	{ "call", prv_parse_call_e, SPECASM_LINE_TYPE_CALL, {0xC4, 0xCD} },
	{ "ccf", NULL, SPECASM_LINE_TYPE_CCF, {0x3F} },
	{ "cp", prv_parse_arith_e, SPECASM_LINE_TYPE_CP, {0xFE, 0xB8} },
	{ "cpd", NULL, SPECASM_LINE_TYPE_CPD, {0xED, 0xA9} },
	{ "cpdr", NULL, SPECASM_LINE_TYPE_CPDR, {0xED, 0xB9} },
	{ "cpi", NULL, SPECASM_LINE_TYPE_CPI, {0xED, 0xA1} },
	{ "cpir", NULL, SPECASM_LINE_TYPE_CPIR, {0xED, 0xB1} },
	{ "cpl", NULL, SPECASM_LINE_TYPE_CPL, {0x2F} },
	{ "daa", NULL, SPECASM_LINE_TYPE_DAA, {0x27} },
	{ "db", prv_parse_db_e, SPECASM_LINE_TYPE_DB, },
	{ "dec", prv_parse_16bit_unary_e, SPECASM_LINE_TYPE_DEC, { 0xB, 0x35} },
	{ "di", NULL, SPECASM_LINE_TYPE_DI, {0xF3} },
	{ "djnz", prv_parse_djnz_e, SPECASM_LINE_TYPE_DJNZ, },
	{ "dw", prv_parse_dw_e, SPECASM_LINE_TYPE_DW, },
	{ "ei", NULL, SPECASM_LINE_TYPE_EI, {0xFB} },
	{ "ex", prv_parse_ex_e, SPECASM_LINE_TYPE_EX, },
	{ "exx", NULL, SPECASM_LINE_TYPE_EXX, {0xD9} },
	{ "halt", NULL, SPECASM_LINE_TYPE_HALT, {0x76} },
	{ "im", prv_parse_im_e, SPECASM_LINE_TYPE_IM, },
	{ "in", prv_parse_in_e, SPECASM_LINE_TYPE_IN, },
	{ "inc", prv_parse_16bit_unary_e, SPECASM_LINE_TYPE_INC, { 0x3, 0x34} },
	{ "ind", NULL, SPECASM_LINE_TYPE_IND, {0xED, 0xAA} },
	{ "indr", NULL, SPECASM_LINE_TYPE_INDR, {0xED, 0xBA} },
	{ "ini", NULL, SPECASM_LINE_TYPE_INI, {0xED, 0xA2} },
	{ "inir", NULL, SPECASM_LINE_TYPE_INIR, {0xED, 0xB2} },
	{ "jp", prv_parse_jp_e, SPECASM_LINE_TYPE_JP, },
	{ "jr", prv_parse_jr_e, SPECASM_LINE_TYPE_JR, {0x20, 0x18} },
	{ "ld", prv_parse_ld_e, SPECASM_LINE_TYPE_LD, },
	{ "ldd", NULL, SPECASM_LINE_TYPE_LDD, {0xED, 0xA8} },
	{ "lddr", NULL, SPECASM_LINE_TYPE_LDDR, {0xED, 0xB8} },
	{ "ldi", NULL, SPECASM_LINE_TYPE_LDI, {0xED, 0xA0} },
	{ "ldir", NULL, SPECASM_LINE_TYPE_LDIR, {0xED, 0xB0} },
	{ "map", NULL, SPECASM_LINE_TYPE_MAP, },
	{ "neg", NULL, SPECASM_LINE_TYPE_NEG, {0xED, 0x44} },
	{ "nop", NULL, SPECASM_LINE_TYPE_NOP, },
	{ "or", prv_parse_arith_e, SPECASM_LINE_TYPE_OR, {0xF6, 0xB0} },
	{ "org", prv_parse_org_e, SPECASM_LINE_TYPE_ORG, },
	{ "otdr", NULL, SPECASM_LINE_TYPE_OTDR, {0xED, 0xBB} },
	{ "otir", NULL, SPECASM_LINE_TYPE_OTIR, {0xED, 0xB3} },
	{ "out", prv_parse_out_e, SPECASM_LINE_TYPE_OUT, },
	{ "outd", NULL, SPECASM_LINE_TYPE_OUTD, {0xED, 0xAB} },
	{ "outi", NULL, SPECASM_LINE_TYPE_OUTI, {0xED, 0xA3} },
	{ "pop", prv_parse_push_pop_e, SPECASM_LINE_TYPE_POP, {0x1} },
	{ "push", prv_parse_push_pop_e, SPECASM_LINE_TYPE_PUSH, {0x5} },
	{ "repb", prv_parse_repb_e, SPECASM_LINE_TYPE_REPB, },
	{ "res", prv_parse_bit_e, SPECASM_LINE_TYPE_RES, {0x80} },
	{ "ret", prv_parse_ret_e, SPECASM_LINE_TYPE_RET, },
	{ "reti", NULL, SPECASM_LINE_TYPE_RETI, {0xED, 0x4D} },
	{ "retn", NULL, SPECASM_LINE_TYPE_RETN, {0xED, 0x45} },
	{ "rl", prv_parse_shift_e, SPECASM_LINE_TYPE_RL, {0x16} },
	{ "rla", NULL, SPECASM_LINE_TYPE_RLA, {0x17} },
	{ "rlc", prv_parse_shift_e, SPECASM_LINE_TYPE_RLC, {0x6} },
	{ "rlca", NULL, SPECASM_LINE_TYPE_RLCA, {0x7} },
	{ "rld", NULL, SPECASM_LINE_TYPE_RLD, {0xED, 0x6F} },
	{ "rr", prv_parse_shift_e, SPECASM_LINE_TYPE_RR, {0x1E} },
	{ "rra", NULL, SPECASM_LINE_TYPE_RRA, {0x1F} },
	{ "rrc", prv_parse_shift_e, SPECASM_LINE_TYPE_RRC, {0xE} },
	{ "rrca", NULL, SPECASM_LINE_TYPE_RRCA, {0xF} },
	{ "rrd", NULL, SPECASM_LINE_TYPE_RRD, {0xED, 0x67} },
	{ "rst", prv_parse_rst_e, SPECASM_LINE_TYPE_RST, },
	{ "sbc", prv_parse_adc_sbc_e, SPECASM_LINE_TYPE_SBC, {0x42, 0x98} },
	{ "scf", NULL, SPECASM_LINE_TYPE_SCF, { 0x37 } },
	{ "set", prv_parse_bit_e, SPECASM_LINE_TYPE_SET, { 0xC0 } },
	{ "sla", prv_parse_shift_e, SPECASM_LINE_TYPE_SLA, {0x26} },
	{ "sra", prv_parse_shift_e, SPECASM_LINE_TYPE_SRA, {0x2E} },
	{ "srl", prv_parse_shift_e, SPECASM_LINE_TYPE_SRL, {0x3E} },
	{ "sub", prv_parse_arith_e, SPECASM_LINE_TYPE_SUB, {0xD6, 0x90} },
	{ "xor", prv_parse_arith_e, SPECASM_LINE_TYPE_XOR, {0xEE, 0xA8} },
};

/* clang-format on */

const static uint8_t opcode_table_size =
    sizeof(opcode_table) / sizeof(specasm_opcode_t);

uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line)
{
	uint8_t m;
	uint8_t l;
	uint8_t r;
	int res;
	const specasm_opcode_t *op_entry;
	char buf[SPECASM_MAX_MNEMOM + 1];
	uint8_t j = 0;

	for (; j < SPECASM_MAX_MNEMOM && str[i] > 32; i++, j++)
		buf[j] = str[i];
	buf[j] = 0;

	l = 0;
	r = opcode_table_size - 1;

	while (l <= r) {
		m = (l + r) >> 1;
		res = strcmp(opcode_table[m].mnemomic, buf);
		if (res < 0) {
			l = m + 1;
		} else if (res > 0) {
			if (m == 0)
				break;
			r = m - 1;
		} else {
			op_entry = &opcode_table[m];
			line->type = op_entry->line_type;
			line->flags = 0;
			memset(&line->data, 0, sizeof(line->data));
			if (!op_entry->fn) {
				line->data.op_code[0] = op_entry->op_code[0];
				line->data.op_code[1] = op_entry->op_code[1];
				if (line->data.op_code[0]) {
					if (line->data.op_code[1])
						line->flags++;
				}
			} else {
				i += op_entry->fn(str + i, line, op_entry);
			}
			return i;
		}
	}

	err_type = SPECASM_ERROR_BAD_MNENOMIC;
	return 0xff;
}

uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf)
{
	uint8_t i;
	char *start = buf;
	const char *name = dump_opcodes[line->type].name;

	for (i = 0; i < 4 && *name; i++)
		*buf++ = *name++;
	if (dump_opcodes[line->type].fn) {
		if (line->type != SPECASM_LINE_TYPE_RET)
			*buf++ = ' ';
		buf += dump_opcodes[line->type].fn(line, buf);
	}
	return buf - start;
}
