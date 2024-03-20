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

#include "line_common.h"
#include "line_parse_common.h"
#include "peer.h"
#include "state.h"

typedef struct specasm_opcode_t_ specasm_opcode_t;

#include "ld_parse.h"

typedef uint8_t (*specasm_parse_fn_t)(const char *args, specasm_line_t *line,
				      const specasm_opcode_t *op_code);

typedef const char *(*specasm_parse_label_fn_t)(const char *args,
						specasm_line_t *line,
						uint8_t *label);

struct specasm_opcode_t_ {
	specasm_parse_fn_t fn;
	uint8_t op_code[2];
};

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

	read = specasm_parse_reg_e(args, &reg, &off, &flags);
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

static uint8_t prv_parse_arith_gen_e(const char *args, specasm_line_t *line,
				     uint8_t areg, uint8_t hl_ind, uint8_t aimm)
{
	uint8_t reg;
	uint8_t off;
	uint8_t read;
	const char *start;
	const char *args2;
	uint8_t flags;
	uint8_t *op_code;
	uint8_t *byte1;
	uint8_t sz = 0;

	start = args;

	op_code = &line->data.op_code[0];
	byte1 = &op_code[1];
	args2 = specasm_parse_byte_imm_or_exp_e(args, line, byte1, byte1);
	if (err_type == SPECASM_ERROR_OK) {
		op_code[0] = aimm;
		sz = 1;
		read = args2 - start;
		goto end;
	} else if ((err_type == SPECASM_ERROR_NUM_TOO_BIG) ||
		   (err_type == SPECASM_ERROR_BAD_EXPRESSION)) {
		return 0;
	}

	err_type = SPECASM_ERROR_OK;
	read = specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	read += args - start;
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

	args = specasm_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (reg == SPECASM_BYTE_REG_A) {
		hl_ind = op_entry->op_code[1] | 6;
		aimm = hl_ind | 0x40;
		areg = op_entry->op_code[1];
		args += prv_parse_arith_gen_e(args, line, areg, hl_ind, aimm);
	} else if (reg == SPECASM_BYTE_REG_HL) {
		args += specasm_parse_reg_e(args, &reg, &off, &flags);
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
	uint16_t val;
	const char *args2;
	uint8_t *op_code;
#endif
	uint8_t opi = 0;
	const char *start = args;
	uint8_t sixbit_regs[] = {SPECASM_BYTE_REG_BC, SPECASM_BYTE_REG_DE,
				 SPECASM_BYTE_REG_HL, SPECASM_BYTE_REG_SP};

	args = specasm_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	switch (reg) {
	case SPECASM_BYTE_REG_A:
		args += prv_parse_arith_gen_e(args, line, 0x80, 0x86, 0xC6);
		return args - start;
	case SPECASM_BYTE_REG_HL:
#ifdef SPECASM_TARGET_NEXT_OPCODES
	case SPECASM_BYTE_REG_BC:
	case SPECASM_BYTE_REG_DE:
#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
	args2 = args + specasm_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		/*
		 * 16 bit immediate cannot be added to ix and iy.
		 */

		if (opi > 0)
			return 0;
		err_type = SPECASM_ERROR_OK;
		args =
		    specasm_parse_word_imm_or_exp_e(args, line, &val, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		op_code = &line->data.op_code[0];
		op_code[0] = 0xED;
		op_code[1] = 0x34 + (2 - (reg - SPECASM_BYTE_REG_BC));
		memcpy(&op_code[2], &val, 2);
		specasm_line_set_size(line, 3);
		specasm_line_set_format(line, flags);
		return args - start;
	} else {
		/*
		 * a cannot be added to ix and add iy.
		 */

		args = args2;
		if (opi == 0) {
			if (reg2 == SPECASM_BYTE_REG_A) {
				op_code = &line->data.op_code[0];
				op_code[0] = 0xED;
				op_code[1] =
				    0x31 + (2 - (reg - SPECASM_BYTE_REG_BC));
				specasm_line_set_size(line, 1);
				return args - start;
			}
			if (reg != SPECASM_BYTE_REG_HL)
				goto bad_reg;
		}
	}
#else
	args += specasm_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
#endif

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

#ifdef SPECASM_TARGET_NEXT_OPCODES
static uint8_t prv_parse_barrel_e(const char *args, specasm_line_t *line,
				  const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t flags;
	uint8_t *op_code;
	const char *start = args;

	args = specasm_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg != SPECASM_BYTE_REG_DE) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	args += specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg != SPECASM_BYTE_REG_B) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	op_code = &line->data.op_code[0];
	op_code[0] = op_entry->op_code[0];
	op_code[1] = op_entry->op_code[1];
	specasm_line_set_size(line, 1);
	return args - start;
}
#endif

static uint8_t prv_parse_org_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t flags;
	uint16_t val;
	const char *start = args;

	args = specasm_get_uword_imm_e(args, &val, &flags);
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

	while (*args == ' ')
		++args;

	if (*args == '=') {
		op_code = &line->data.op_code[0];
		args = specasm_get_exp_e(line, args, &op_code[2]);
		val = 0;
	} else {
		args = specasm_get_byte_imm_e(args, &val, &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (val > 7) {
			err_type = SPECASM_ERROR_BAD_NUM;
			return 0;
		}
	}

	while (*args == ' ')
		++args;

	if (*args != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	args++;
	args += specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if ((reg > SPECASM_BYTE_REG_A) && (reg != SPECASM_BYTE_REG_HL_IND) &&
	    (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)) {
		err_type = SPECASM_ERROR_BAD_EXPRESSION;
		return 0;
	}

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

	args = specasm_parse_jump_label_e(args, line, &line->data.op_code[1]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	line->data.op_code[0] = 0x10;
	specasm_line_set_size(line, 1);

	return args - start;
}

static uint8_t prv_signed_ok(uint8_t flags, uint8_t val)
{
	return (flags == SPECASM_FLAGS_NUM_SIGNED) ||
	       ((flags == SPECASM_FLAGS_NUM_UNSIGNED) && (val < 128));
}

static const char *prv_parse_data_exp_e(const char *args, specasm_line_t *line,
					uint8_t size)
{
	args = specasm_get_exp_e(line, args - 1, &line->data.op_code[0]);
	specasm_line_set_size(line, size);
	return args;
}

static uint8_t prv_parse_db_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	uint8_t val;
	uint8_t flags;
	uint8_t flags2;
	uint8_t signed_ok;
	const char *args2;
	uint8_t i = 0;
	const char *start = args;

	while (*args == ' ')
		++args;

	if (*args == '=') {
		args = prv_parse_data_exp_e(args + 1, line, 0);
		return args - start;
	}

	args = specasm_get_byte_imm_e(args, &val, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	line->data.op_code[0] = val;

	for (i = 1; i < 4; i++) {
		args2 = args;
		while (*args2 == ' ')
			++args2;
		if (*args2 != ',')
			break;
		signed_ok = prv_signed_ok(flags, val);
		args = specasm_get_byte_imm_e(args2 + 1, &val, &flags2);
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
	uint8_t *op_code;
	const char *args2 = args;
	int8_t sz = 1;
	const char *start = args;

	while (*args == ' ')
		++args;

	if (*args == '=') {
		args = prv_parse_data_exp_e(args + 1, line, 1);
		return args - start;
	}

	args = specasm_get_word_imm_e(args, &val, &flags);
	if (err_type == SPECASM_ERROR_NUM_TOO_BIG)
		return 0;
	if (err_type == SPECASM_ERROR_OK) {
		*((uint16_t *)&line->data.op_code[0]) = val;
		args2 = args;
		while (*args2 == ' ')
			++args2;
		if (*args2 == ',') {
			signed_ok = prv_signed_ok_u16(flags, val);
			args = specasm_get_word_imm_e(args2 + 1, &val, &flags2);
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
		while (*args2 == ' ')
			++args2;
		args = specasm_parse_jump_label_e(args2, line, op_code);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
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

	args2 = specasm_parse_reg_comma_e(args, &reg1, &off1, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	args2 += specasm_parse_reg_e(args2, &reg2, &off2, &flags);
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
	uint8_t val;
	const char *args2;

	args2 = specasm_parse_byte_imm_or_exp_e(args, line, &val,
						&line->data.op_code[2]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

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

	args = specasm_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (reg > SPECASM_BYTE_REG_A) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	args2 = args + specasm_parse_reg_e(args, &reg2, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		if (reg != SPECASM_BYTE_REG_A)
			return 0;
		err_type = SPECASM_ERROR_OK;
		args = specasm_get_byte_imm_ind_e(line, args, &val);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
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

	read = specasm_parse_reg_e(args, &reg, &off, &flags);
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
		case SPECASM_BYTE_REG_C_IND:
			op1 = 0xED;
			op2 = 0x98;
			break;
#endif
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

	args = specasm_parse_label_or_exp_e(args, line, &line->data.op_code[1]);
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
					uint8_t max_cc,
					specasm_parse_label_fn_t fn_e)
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

	args2 = fn_e(args, line, &label);

	/* If the label's bad we might have a 16 bit offset so return
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

	len = prv_parse_relative_jmp_e(args, line, op_entry, max_cc,
				       specasm_parse_label_or_exp_e);
	if (err_type != SPECASM_ERROR_BAD_LABEL)
		return len;

	start = args;
	err_type = SPECASM_ERROR_OK;
	args = specasm_get_uword_imm_e(args + len, &val, &flags);
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
	return prv_parse_relative_jmp_e(args, line, op_entry, SPECASM_CC_C,
					specasm_parse_jump_label_e);
}

static uint8_t prv_parse_ld_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	return specasm_parse_ld_e(args, line, op_entry);
}

#ifdef SPECASM_TARGET_NEXT_OPCODES
static uint8_t prv_parse_mirror_e(const char *args, specasm_line_t *line,
				  const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t flags;
	uint8_t *op_code;
	const char *start = args;

	args += specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg != SPECASM_BYTE_REG_A) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}
	op_code = &line->data.op_code[0];
	op_code[0] = 0xED;
	op_code[1] = 0x24;
	specasm_line_set_size(line, 1);
	return args - start;
}

static uint8_t prv_parse_mul_e(const char *args, specasm_line_t *line,
			       const specasm_opcode_t *op_entry)
{
	uint8_t reg;
	uint8_t off;
	uint8_t flags;
	uint8_t *op_code;
	const char *start = args;

	args = specasm_parse_reg_comma_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg != SPECASM_BYTE_REG_D) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	args += specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;
	if (reg != SPECASM_BYTE_REG_E) {
		err_type = SPECASM_ERROR_BAD_REG;
		return 0;
	}

	op_code = &line->data.op_code[0];
	op_code[0] = 0xED;
	op_code[1] = 0x30;
	specasm_line_set_size(line, 1);
	return args - start;
}

static uint8_t prv_parse_nbrk_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	line->data.op_code[0] = 0xED;
	line->data.op_code[1] = 0x91;
	line->data.op_code[2] = 2;
	line->data.op_code[3] = 8;
	specasm_line_set_size(line, 3);
	return 0;
}

static uint8_t prv_parse_nextreg_e(const char *args, specasm_line_t *line,
				   const specasm_opcode_t *op_entry)
{
	const char *args2;
	uint8_t off;
	uint8_t flags;
	uint8_t reg;
	const char *start = args;
	uint8_t *ptr = &line->data.op_code[2];

	args = specasm_parse_byte_imm_or_exp_e(args, line, ptr, ptr);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	while (*args == ' ')
		++args;
	if (*args++ != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	args2 = args + specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SPECASM_ERROR_OK;
		args = specasm_get_byte_imm_e(args, &line->data.op_code[3],
					      &flags);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		specasm_line_set_format2(line, flags);
		line->data.op_code[1] = 0x91;
		specasm_line_set_size(line, 3);
	} else {
		if (reg != SPECASM_BYTE_REG_A) {
			err_type = SPECASM_ERROR_BAD_REG;
			return 0;
		}
		args = args2;
		line->data.op_code[1] = 0x92;
		specasm_line_set_size(line, 2);
	}
	line->data.op_code[0] = 0xED;
	;
	return args - start;
}
#endif

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

	args2 = args + specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
		err_type = SPECASM_ERROR_OK;
		args = specasm_get_byte_imm_ind_e(line, args, &val);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
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

	args += specasm_parse_reg_e(args, &reg2, &off, &flags);
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
	const char *start;
	uint16_t val;
	uint8_t val_bytes[2];
#endif
	uint8_t *op_code;
	uint8_t sz = 1;
	uint8_t ainstr = op_entry->op_code[0];

	read = specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK) {
#ifdef SPECASM_TARGET_NEXT_OPCODES
		if (line->type == SPECASM_LINE_TYPE_PUSH) {
			err_type = SPECASM_ERROR_OK;
			start = args;
			args = specasm_parse_word_imm_or_exp_e(args, line, &val,
							       &flags);
			if (err_type == SPECASM_ERROR_BAD_NUM)
				err_type = SPECASM_ERROR_BAD_REG;
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			memcpy(&val_bytes[0], &val, 2);
			op_code = &line->data.op_code[0];
			op_code[0] = 0xED;
			op_code[1] = 0x8A;
			op_code[2] = val_bytes[1];
			op_code[3] = val_bytes[0];
			specasm_line_set_size(line, 3);
			specasm_line_set_format(line, flags);
			return args - start;
		}
#endif
		return 0;
	}

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
	const char *start = args;

	args = specasm_parse_byte_imm_or_exp_e(args, line, &val,
					       &line->data.op_code[1]);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if ((val > 0x38) || (val & 7)) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	line->data.op_code[0] = 0xC7 | val;

	return args - start;
}

static uint8_t prv_parse_ds_e(const char *args, specasm_line_t *line,
			      const specasm_opcode_t *op_entry)
{
	uint16_t *count;
	uint8_t flags;
	const char *args2;
	uint8_t *op_code = &line->data.op_code[0];

	/*
	 * We're storing the count and val in the opcode in
	 * reverse order to which they are parsed.  This is
	 * to maintain backward compatibility with Specasm v1,
	 * which had the repb directive that expected its
	 * arguments in the reverse order.
	 *
	 * So val is stored in the first byte and count in the
	 * second and third.
	 */

	count = (uint16_t *)(&op_code[1]);
	args2 = specasm_get_uword_imm_e(args, count, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (*count == 0) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}
	specasm_line_set_format2(line, flags);

	while (*args2 == ' ')
		++args2;

	if (*args2 != ',') {
		err_type = SPECASM_ERROR_COMMA_EXPECTED;
		return 0;
	}

	args2 = specasm_get_byte_imm_e(args2 + 1, op_code, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	specasm_line_set_format(line, flags);

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

	read = specasm_parse_reg_e(args, &reg, &off, &flags);
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
static uint8_t prv_parse_test_e(const char *args, specasm_line_t *line,
				const specasm_opcode_t *op_entry)
{
	uint8_t *op_code;
	const char *args2;

	op_code = &line->data.op_code[0];
	op_code[0] = 0xED;
	op_code[1] = 0x27;
	args2 = specasm_parse_byte_imm_or_exp_e(args, line, &op_code[2],
						&op_code[2]);
	specasm_line_set_size(line, 2);

	return args2 - args;
}
#endif

static uint8_t prv_parse_align_e(const char *args, specasm_line_t *line,
				 const specasm_opcode_t *op_entry)
{
	long lval;
	uint8_t flags;
	uint16_t target;
	uint8_t i;
	uint16_t valid;
	const char *start = args;

	args = specasm_get_long_imm_e(args, &lval, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	valid = 1;
	target = (uint16_t)lval;
	for (i = 1; i <= 8; i++) {
		valid = valid << 1;
		if (valid == target)
			break;
	}

	if (i > 8) {
		err_type = SPECASM_ERROR_BAD_NUM;
		return 0;
	}

	specasm_line_set_format(line, flags);
	line->data.op_code[0] = i;

	return args - start;
}

/* clang-format off */

static const specasm_opcode_t opcode_table[] = {
	{ prv_parse_adc_sbc_e, {0x4A, 0x88} },        // SPECASM_LINE_TYPE_ADC
	{ prv_parse_add_e, },                         // SPECASM_LINE_TYPE_ADD
	{ prv_parse_align_e, },                       // SPECASM_LINE_TYPE_ALIGN
	{ prv_parse_arith_e, {0xE6, 0xA0} },          // SPECASM_LINE_TYPE_AND
	{ prv_parse_bit_e, { 0x40 } },                // SPECASM_LINE_TYPE_BIT
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ prv_parse_barrel_e, { 0xED, 0x2C } },       // SPECASM_LINE_TYPE_BRLC
	{ prv_parse_barrel_e, { 0xED, 0x28 } },       // SPECASM_LINE_TYPE_BSLA
	{ prv_parse_barrel_e, { 0xED, 0x29 } },       // SPECASM_LINE_TYPE_BSRA
	{ prv_parse_barrel_e, { 0xED, 0x2B } },       // SPECASM_LINE_TYPE_BSRF
	{ prv_parse_barrel_e, { 0xED, 0x2A } },       // SPECASM_LINE_TYPE_BSRL
#endif
	{ prv_parse_call_e, {0xC4, 0xCD} },           // SPECASM_LINE_TYPE_CALL
	{ NULL, {0x3F} },                             // SPECASM_LINE_TYPE_CCF
	{ prv_parse_arith_e, {0xFE, 0xB8} },          // SPECASM_LINE_TYPE_CP
	{ NULL, {0xED, 0xA9} },                       // SPECASM_LINE_TYPE_CPD
	{ NULL, {0xED, 0xB9} },                       // SPECASM_LINE_TYPE_CPDR
	{ NULL, {0xED, 0xA1} },                       // SPECASM_LINE_TYPE_CPI
	{ NULL, {0xED, 0xB1} },                       // SPECASM_LINE_TYPE_CPIR
	{ NULL, {0x2F} },                             // SPECASM_LINE_TYPE_CPL
	{ NULL, {0x27} },                             // SPECASM_LINE_TYPE_DAA
	{ prv_parse_db_e, },                          // SPECASM_LINE_TYPE_DB
	{ prv_parse_16bit_unary_e, { 0xB, 0x35} },    // SPECASM_LINE_TYPE_DEC
	{ NULL, {0xF3} },                             // SPECASM_LINE_TYPE_DI
	{ prv_parse_djnz_e, },                        // SPECASM_LINE_TYPE_DJNZ
	{ prv_parse_ds_e, },                          // SPECASM_LINE_TYPE_DS
	{ prv_parse_dw_e, },                          // SPECASM_LINE_TYPE_DW
	{ NULL, {0xFB} },                             // SPECASM_LINE_TYPE_EI
	{ prv_parse_ex_e, },                          // SPECASM_LINE_TYPE_EX
	{ NULL, {0xD9} },                             // SPECASM_LINE_TYPE_EXX
	{ NULL, {0x76} },                             // SPECASM_LINE_TYPE_HALT
	{ prv_parse_im_e, },                          // SPECASM_LINE_TYPE_IM
	{ prv_parse_in_e, },                          // SPECASM_LINE_TYPE_IN
	{ prv_parse_16bit_unary_e, { 0x3, 0x34} },    // SPECASM_LINE_TYPE_INC
	{ NULL, {0xED, 0xAA} },                       // SPECASM_LINE_TYPE_IND
	{ NULL, {0xED, 0xBA} },                       // SPECASM_LINE_TYPE_INDR
	{ NULL, {0xED, 0xA2} },                       // SPECASM_LINE_TYPE_INI
	{ NULL, {0xED, 0xB2} },                       // SPECASM_LINE_TYPE_INIR
	{ prv_parse_jp_e, },                          // SPECASM_LINE_TYPE_JP
	{ prv_parse_jr_e, {0x20, 0x18} },             // SPECASM_LINE_TYPE_JR
	{ prv_parse_ld_e, },                          // SPECASM_LINE_TYPE_LD
	{ NULL, {0xED, 0xA8} },                       // SPECASM_LINE_TYPE_LDD
	{ NULL, {0xED, 0xB8} },                       // SPECASM_LINE_TYPE_LDDR
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL, {0xED, 0xBC} },                       // SPECASM_LINE_TYPE_LDDRX
	{ NULL, {0xED, 0xAC} },                       // SPECASM_LINE_TYPE_LDDX
#endif
	{ NULL, {0xED, 0xA0} },                       // SPECASM_LINE_TYPE_LDI
	{ NULL, {0xED, 0xB0} },                       // SPECASM_LINE_TYPE_LDIR
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL, {0xED, 0xB4} },                       // SPECASM_LINE_TYPE_LDIRX
	{ NULL, {0xED, 0xA4} },                       // SPECASM_LINE_TYPE_LDIX
	{ NULL, {0xED, 0xB7} },                       // SPECASM_LINE_TYPE_LDPIRX
	{ NULL, {0xED, 0xA5} },                       // SPECASM_LINE_TYPE_LDWS
#endif
	{ NULL, },                                    // SPECASM_LINE_TYPE_MAP
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ prv_parse_mirror_e, },                      // SPECASM_LINE_TYPE_MIRROR
	{ prv_parse_mul_e, },                         // SPECASM_LINE_TYPE_MUL
	{ prv_parse_nbrk_e, },                        // SPECASM_LINE_TYPE_NBRK
#endif
	{ NULL, {0xED, 0x44} },                       // SPECASM_LINE_TYPE_NEG
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ prv_parse_nextreg_e, },                     // SPECASM_LINE_TYPE_NEXTREG
#endif
	{ NULL, },                                    // SPECASM_LINE_TYPE_NOP
	{ prv_parse_arith_e, {0xF6, 0xB0} },          // SPECASM_LINE_TYPE_OR
	{ prv_parse_org_e, },                         // SPECASM_LINE_TYPE_ORG
	{ NULL, {0xED, 0xBB} },                       // SPECASM_LINE_TYPE_OTDR
	{ NULL, {0xED, 0xB3} },                       // SPECASM_LINE_TYPE_OTIR
	{ prv_parse_out_e, },                         // SPECASM_LINE_TYPE_OUT
	{ NULL, {0xED, 0xAB} },                       // SPECASM_LINE_TYPE_OUTD
	{ NULL, {0xED, 0xA3} },                       // SPECASM_LINE_TYPE_OUTI
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL, {0xED, 0x90} },                       // SPECASM_LINE_TYPE_OUTINB
	{ NULL, {0xED, 0x94} },                       // SPECASM_LINE_TYPE_PIXELAD
	{ NULL, {0xED, 0x93} },                       // SPECASM_LINE_TYPE_PIXELDN
#endif
	{ prv_parse_push_pop_e, {0x1} },              // SPECASM_LINE_TYPE_POP
	{ prv_parse_push_pop_e, {0x5} },              // SPECASM_LINE_TYPE_PUSH
	{ prv_parse_bit_e, {0x80} },                  // SPECASM_LINE_TYPE_RES
	{ prv_parse_ret_e, },                         // SPECASM_LINE_TYPE_RET
	{ NULL, {0xED, 0x4D} },                       // SPECASM_LINE_TYPE_RETI
	{ NULL, {0xED, 0x45} },                       // SPECASM_LINE_TYPE_RETN
	{ prv_parse_shift_e, {0x16} },                // SPECASM_LINE_TYPE_RL
	{ NULL, {0x17} },                             // SPECASM_LINE_TYPE_RLA
	{ prv_parse_shift_e, {0x6} },                 // SPECASM_LINE_TYPE_RLC
	{ NULL, {0x7} },                              // SPECASM_LINE_TYPE_RLCA
	{ NULL, {0xED, 0x6F} },                       // SPECASM_LINE_TYPE_RLD
	{ prv_parse_shift_e, {0x1E} },                // SPECASM_LINE_TYPE_RR
	{ NULL, {0x1F} },                             // SPECASM_LINE_TYPE_RRA
	{ prv_parse_shift_e, {0xE} },                 // SPECASM_LINE_TYPE_RRC
	{ NULL, {0xF} },                              // SPECASM_LINE_TYPE_RRCA
	{ NULL, {0xED, 0x67} },                       // SPECASM_LINE_TYPE_RRD
	{ prv_parse_rst_e, },                         // SPECASM_LINE_TYPE_RST
	{ prv_parse_adc_sbc_e, {0x42, 0x98} },        // SPECASM_LINE_TYPE_SBC
	{ NULL, { 0x37 } },                           // SPECASM_LINE_TYPE_SCF
	{ prv_parse_bit_e, { 0xC0 } },                // SPECASM_LINE_TYPE_SET
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL, { 0xED, 0x95 } },                     // SPECASM_LINE_TYPE_SETAE
#endif
	{ prv_parse_shift_e, {0x26} },                // SPECASM_LINE_TYPE_SLA
	{ prv_parse_shift_e, {0x2E} },                // SPECASM_LINE_TYPE_SRA
	{ prv_parse_shift_e, {0x3E} },                // SPECASM_LINE_TYPE_SRL
	{ prv_parse_arith_e, {0xD6, 0x90} },          // SPECASM_LINE_TYPE_SUB
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL, {0xED, 0x23} },                       // SPECASM_LINE_TYPE_SWAPNIB
	{ prv_parse_test_e, },                        // SPECASM_LINE_TYPE_TEST
#endif
	{ prv_parse_arith_e, {0xEE, 0xA8} },          // SPECASM_LINE_TYPE_XOR
	{ NULL, },                                    // SPECASM_LINE_TYPE_ZX81
};

/* clang-format on */

static const uint8_t opcode_table_size =
    sizeof(opcode_table) / sizeof(specasm_opcode_t);

#ifdef SPECASM_NEXT_BANKED
uint8_t specasm_parse_exp_banked_e(const char *str, uint8_t *label1,
				   uint8_t *label1_type)
#else
uint8_t specasm_parse_exp_e(const char *str, uint8_t *label1,
			    uint8_t *label1_type)
#endif
{
	uint8_t j;
	uint8_t len;
	uint8_t i;
	uint8_t c;
	uint8_t brackets = 0;
	uint8_t end = SPECASM_LINE_MAX_LEN - 1;

	for (i = 0; str[i] == ' '; i++)
		;

	end = i;
	c = str[end];
	while (c) {
		if ((c == ',') || (c == ';'))
			break;
		if (c == ')') {
			if (brackets == 0)
				break;
			else
				brackets--;
		} else if (c == '(') {
			brackets++;
		}
		end++;
		c = str[end];
	}
	if ((i == end) || brackets) {
		err_type = SPECASM_ERROR_BAD_EXPRESSION;
		return 0;
	}

	end--;

	while ((end > i) && (str[end] == ' '))
		end--;

	len = ((end - i) + 1);
	for (j = 0; j < len; j++)
		scratch[j] = str[i + j];
	scratch[j] = 0;

	if (len < SPECASM_MAX_SHORT_LEN) {
		*label1 = specasm_state_add_short_e(scratch);
		*label1_type = SPECASM_FLAGS_ADDR_SHORT;
	} else {
		*label1 = specasm_state_add_long_e(scratch);
		*label1_type = SPECASM_FLAGS_ADDR_LONG;
	}

	return end + 1;
}

#ifdef SPECASM_NEXT_BANKED
uint8_t specasm_parse_mnemomic_banked_e(const char *str, uint8_t i,
					specasm_line_t *line)
#else
uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line)
#endif
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
		res = strcmp(mnemomics_table[m].mnemomic, buf);
		if (res < 0) {
			l = m + 1;
		} else if (res > 0) {
			if (m == 0)
				break;
			r = m - 1;
		} else {
			op_entry = &opcode_table[m];
			line->type = mnemomics_table[m].line_type;
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
