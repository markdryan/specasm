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

static const char *prv_get_word_imm_ind_e(const char *args, uint16_t *val,
					  uint8_t *flags)
{
	while (*args == ' ')
		++args;

	if (*args != '(') {
		err_type = SPECASM_ERROR_BAD_NUM;
		return NULL;
	}
	args = specasm_get_uword_imm_e(args + 1, val, flags);
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

	args = specasm_parse_label_or_exp_e(args, line, val);
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

	args += specasm_parse_reg_e(args, &reg, &off, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_set_ld_imm_ind_e(line, val, reg, 0);

	return args;
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

uint8_t specasm_parse_ld_e(const char *args, specasm_line_t *line,
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
	uint8_t v;
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
		if ((err_type == SPECASM_ERROR_BAD_EXPRESSION) ||
		    (err_type == SPECASM_ERROR_TOO_MANY_SHORT_STRINGS) ||
		    (err_type == SPECASM_ERROR_TOO_MANY_LONG_STRINGS))
			return 0;
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
	args += specasm_parse_reg_e(args, &reg, &off, &flags);
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
	    (err_type == SPECASM_ERROR_NUM_NEG) ||
	    (err_type == SPECASM_ERROR_TOO_MANY_SHORT_STRINGS) ||
	    (err_type == SPECASM_ERROR_TOO_MANY_LONG_STRINGS)) {
		return 0;
	} else if (err_type == SPECASM_ERROR_OK) {
		specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
		specasm_line_set_format(line, flags);
	} else {
		err_type = SPECASM_ERROR_OK;
		args2 = prv_get_label_ind_e(args, line, &label);
		if ((err_type == SPECASM_ERROR_BAD_EXPRESSION) ||
		    (err_type == SPECASM_ERROR_TOO_MANY_SHORT_STRINGS) ||
		    (err_type == SPECASM_ERROR_TOO_MANY_LONG_STRINGS))
			return 0;
		val = label;
	}
	if (err_type == SPECASM_ERROR_OK) {
		prv_set_ld_imm_ind_e(line, val, reg, 0x8);
		return args2 - start;
	}
	err_type = SPECASM_ERROR_OK;

	while (*args == ' ')
		++args;
	args2 = specasm_parse_word_imm_or_exp_e(args, line, &val, &flags2);
	if ((err_type == SPECASM_ERROR_NUM_TOO_BIG) ||
	    (err_type == SPECASM_ERROR_BAD_EXPRESSION) ||
	    (err_type == SPECASM_ERROR_TOO_MANY_SHORT_STRINGS) ||
	    (err_type == SPECASM_ERROR_TOO_MANY_LONG_STRINGS)) {
		return 0;
	} else if (err_type == SPECASM_ERROR_OK) {
		prv_ld_imm_e(line, reg, off, flags, flags2, val);
		return args2 - start;
	} else if ((reg >= SPECASM_BYTE_REG_BC) &&
		   (reg <= SPECASM_BYTE_REG_IY)) {
		/*
		 * For 16 bit immediate loads we allow labels in place of
		 * the 16 bit value.
		 */

		err_type = SPECASM_ERROR_OK;
		args2 = specasm_parse_jump_label_e(args, line, &v);
		val = v;
		if (err_type == SPECASM_ERROR_OK) {
			prv_ld_16bit_imm_e(line, reg,
					   specasm_line_get_format(line), val);
			return args2 - start;
		}
	}

	err_type = SPECASM_ERROR_OK;
	args += specasm_parse_reg_e(args, &reg2, &off2, &flags);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_ld_reg_reg_e(line, reg, off, reg2, off2, flags);

	return args - start;
}
