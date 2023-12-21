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
#include "peer.h"
#include "state.h"

static char *prv_dump_exp_e(const specasm_line_t *line, char *buf, uint8_t id);

typedef uint8_t (*specasm_dump_opcode_fn_t)(const specasm_line_t *line,
					    char *buf);

struct specasm_line_opcode_dump_t_ {
	specasm_dump_opcode_fn_t fn;
	uint8_t index;
};

typedef struct specasm_line_opcode_dump_t_ specasm_line_opcode_dump_t;

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
		    specasm_dump_index(op_code, buf, specasm_line_get_format(line));
	} else if (op_code[0] == imm) {

		if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
			buf = prv_dump_exp_e(line, buf, op_code[1]);
		else
			buf += specasm_dump_byte(buf, op_code[1],
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
			buf = specasm_dump_index(op_code, buf,
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
		buf += specasm_dump_word(buf, id, specasm_line_get_format(line));
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

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
		*buf++ = '=';

	return prv_dump_jump_label_fmt_e(line, buf, id, fmt);
}

static char *prv_dump_exp_e(const specasm_line_t *line, char *buf, uint8_t id)
{
	uint8_t fmt = specasm_line_get_addr_type(line);
	*buf++ = '=';
	return prv_dump_jump_label_fmt_e(line, buf, id, fmt);
}

static char *prv_dump_cc(const uint8_t cc, char *buf)
{
#ifndef SPECASM_NEXT_BANKED
	static const char *codes[] = {"nz", "z",  "nc", "c",
				      "po", "pe", "p",  "m"};
#else
	const char *codes[] = {"nz", "z",  "nc", "c",
		"po", "pe", "p",  "m"};
#endif
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
#ifdef SPECASM_TARGET_NEXT
	uint16_t val;
	uint8_t reg2 = 0;
#else
	uint8_t reg2;
#endif
	const char *code;
#ifndef SPECASM_NEXT_BANKED
	static const char *codes[] = {"bc", "de", "hl", "sp", "ix", "iy"};
#else
	const char *codes[] = {"bc", "de", "hl", "sp", "ix", "iy"};
#endif
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
#ifdef SPECASM_TARGET_NEXT
	} else if (op_code[0] == 0xED) {
		if (op_code[1] <= 0x33)
			reg1 = 0x31;
		else
			reg1 = 0x34;
		reg1 = 2 - (op_code[1] - reg1);
#endif
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
#ifdef SPECASM_TARGET_NEXT
	if (op_code[0] == 0xED) {
		if (op_code[1] <= 0x33) {
			buf[0] = 'a';
			buf++;
		} else {
			memcpy(&val, &op_code[2], 2);
			if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
				buf = prv_dump_jump_label_e(line, buf, val);
			else
				buf += specasm_dump_word(
				    buf, val, specasm_line_get_format(line));
		}
		return buf - start;
	}
#endif
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

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ) {
		buf = prv_dump_exp_e(line, buf, op_code[2]);
	} else {
		val = (index) ? op_code[3] : op_code[1];
		(void)itoa((val >> 3) & 7, buf, 10);
		buf += strlen(buf);
	}
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	if (op_code[0] == 0xCB)
		buf = prv_byte_and_hl_ind(op_code[1], buf);
	else
		buf =
		    specasm_dump_index(op_code, buf, specasm_line_get_format(line));

	return buf - start;
}

#ifdef SPECASM_TARGET_NEXT
static uint8_t prv_dump_barrel_e(const specasm_line_t *line, char *buf)
{
	buf[0] = 'd';
	buf[1] = 'e';
	buf[2] = ',';
	buf[3] = ' ';
	buf[4] = 'b';

	return 5;
}
#endif

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
		buf += specasm_dump_word(buf, val, specasm_line_get_format(line));
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
#ifndef SPECASM_NEXT_BANKED
	static const char *com[] = {
#else
	const char *com[] = {
#endif
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
	char *start;
	const uint8_t *op_code = line->data.op_code;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ) {
		start = buf;
		buf = prv_dump_exp_e(line, buf, op_code[2]);
		return buf - start;
	}

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

static char *specasm_dump_byte_imm_ind_e(const specasm_line_t *line, char *buf,
				     uint8_t index)
{
	const uint8_t *op_code = line->data.op_code;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
		return prv_dump_exp_e(line, buf, op_code[index]);

	return buf + specasm_dump_byte(buf, op_code[index],
				   specasm_line_get_format(line));
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
	if (op_code[0] == 0xDB)
		buf = specasm_dump_byte_imm_ind_e(line, buf, 1);
	else
		*buf++ = 'c';
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
#ifndef SPECASM_NEXT_BANKED
	static const char *com[] = {
#else
	const char *com[] = {
#endif
	    "\xe9"
	    "(hl)",
	    "\xdd"
	    "(ix)",
	    "\xfd"
	    "(iy)",
#ifdef SPECASM_TARGET_NEXT
	    "\xed"
	    "(c)",
#endif
	};
	const uint8_t *op_code = line->data.op_code;

	switch (op_code[0]) {
	case 0xDD:
	case 0xFD:
	case 0xE9:
#ifdef SPECASM_TARGET_NEXT
	case 0xED:
#endif
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
#ifndef SPECASM_NEXT_BANKED
	static const char *com[] = {
#else
	const char *com[] = {
#endif
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
#ifndef SPECASM_NEXT_BANKED
	static const char *codes[] = {"bc", "de", "hl",  "sp",
				      "ix", "iy", "(hl)"};
#else
	const char *codes[] = {"bc", "de", "hl",  "sp",
			       "ix", "iy", "(hl)"};
#endif
	code = codes[reg];
	while (*code)
		*buf++ = *code++;

	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;
	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ ||
	    specasm_line_get_addr_type(line)) {
		buf = prv_dump_jump_label_e(line, buf, val);
	} else {
		buf += specasm_dump_word(buf, val, specasm_line_get_format(line));
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
#ifndef SPECASM_NEXT_BANKED
	static const char *com[] = {
#else
	const char *com[] = {
#endif
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
	return specasm_dump_index(line->data.op_code, buf + 3,
			      specasm_line_get_format(line));
}

static char *prv_dump_ld_ind_off_reg(const specasm_line_t *line, char *buf)
{
	buf = specasm_dump_index(line->data.op_code, buf,
			     specasm_line_get_format(line));
	buf[0] = ',';
	buf[1] = ' ';
	buf[2] = byte_regs[line->data.op_code[1] & 7];

	return buf + 3;
}

static char *prv_dump_ld_ind_off_imm(const specasm_line_t *line, char *buf)
{
	buf = specasm_dump_index(line->data.op_code, buf,
			     specasm_line_get_format(line));
	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;
	buf += specasm_dump_byte(buf, line->data.op_code[3],
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
#ifndef SPECASM_NEXT_BANKED
	static const char *com[] = {
#else
	const char *com[] = {
#endif
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
		if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
			buf = prv_dump_exp_e(line, buf, line->data.op_code[1]);
		else
			buf += specasm_dump_byte(buf, line->data.op_code[1],
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

	buf[0] = '=';
	str = prv_dump_jump_label_fmt_e(line, buf + 1,
					line->data.op_code[id_buf],
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

#ifdef SPECASM_TARGET_NEXT
static uint8_t prv_dump_mirror_e(const specasm_line_t *line, char *buf)
{
	buf[0] = 'a';

	return 1;
}

static uint8_t prv_dump_mul_e(const specasm_line_t *line, char *buf)
{
	buf[0] = 'd';
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = 'e';

	return 4;
}

static uint8_t prv_dump_nextreg_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
		buf = prv_dump_exp_e(line, buf, line->data.op_code[2]);
	else
		buf += specasm_dump_byte(buf, line->data.op_code[2],
				     specasm_line_get_format(line));

	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	if (line->data.op_code[1] == 0x92) {
		buf[0] = 'a';
		buf++;
	} else {
		buf += specasm_dump_byte(buf, line->data.op_code[3],
				     specasm_line_get_format2(line));
	}

	return buf - start;
}
#endif

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
	if (op_code[0] == 0xD3)
		buf = specasm_dump_byte_imm_ind_e(line, buf, 1);
	else
		*buf++ = 'c';
	buf[0] = ')';
	buf[1] = ',';
	buf[2] = ' ';
	buf[3] = byte_regs[code];

	return (buf + 4) - start;
}

static uint8_t prv_dump_rst_e(const specasm_line_t *line, char *buf)
{
	char *start;
	const uint8_t *op_code = line->data.op_code;
	uint8_t val = op_code[0] & 0x38;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ) {
		start = buf;
		buf = prv_dump_exp_e(line, buf, op_code[1]);
		return buf - start;
	}

	return specasm_dump_byte(buf, val, specasm_line_get_format(line));
}

static uint8_t prv_dump_sbc_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_adc_sbc_e(line, buf, 0xDE, 0x98);
}

static uint8_t prv_dump_stack(const specasm_line_t *line, char *buf)
{
#ifdef SPECASM_TARGET_NEXT
	uint16_t val;
	char *start;
#endif
	const uint8_t *op_code = line->data.op_code;
	char a = 'a';
	char b = 'f';

	if (op_code[0] == 0xFD) {
		a = 'i';
		b = 'y';
	} else if (op_code[0] == 0xDD) {
		a = 'i';
		b = 'x';
#ifdef SPECASM_TARGET_NEXT
	} else if (op_code[0] == 0xED) {
		start = buf;
		memcpy(&val, &op_code[2], 2);
		if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
			buf = prv_dump_jump_label_e(line, buf, val);
		else
			buf += specasm_dump_word(buf, val,
					     specasm_line_get_format(line));
		return buf - start;
#endif
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
		    specasm_dump_index(op_code, buf, specasm_line_get_format(line));
	else
		err_type = SPECASM_ERROR_BAD_REG;

	return buf - start;
}

static uint8_t prv_dump_sub_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	return prv_dump_arith_e(line, buf, 0xD6, 0x90) - start;
}

#ifdef SPECASM_TARGET_NEXT
static uint8_t prv_dump_test_e(const specasm_line_t *line, char *buf)
{
	char *start = buf;

	buf = specasm_dump_byte_imm_ind_e(line, buf, 2);

	return buf - start;
}
#endif

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
	uint8_t sz;
	char *end;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ) {
		end = prv_dump_exp_e(line, buf, line->data.op_code[0]);
		return end - buf;
	}

	sz = specasm_line_get_size(line) + 1;
	len = specasm_dump_byte(buf, line->data.op_code[0],
			    specasm_line_get_format(line));
	for (i = 1; i < sz; i++) {
		ptr = &buf[len];
		ptr[0] = ',';
		len += specasm_dump_byte(ptr + 1, line->data.op_code[i],
				     specasm_line_get_format(line));
		len++;
	}
	return len;
}

static uint8_t prv_dump_equw_e(const specasm_line_t *line, char *buf)
{
	char *str;
	char *end;
	uint16_t id;

	if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ) {
		end = prv_dump_exp_e(line, buf, line->data.op_code[0]);
		return end - buf;
	}

	if (specasm_line_get_addr_type(line)) {
		str = prv_dump_jump_label_e(line, buf, line->data.op_code[0]);
	} else {
		id = *((uint16_t *)&line->data.op_code[0]);
		str =
		    buf + specasm_dump_word(buf, id, specasm_line_get_format(line));
		if (specasm_line_get_size(line) > 1) {
			str[0] = ',';
			str++;
			id = *((uint16_t *)&line->data.op_code[2]);
			str += specasm_dump_word(str, id,
					     specasm_line_get_format(line));
		}
	}
	return str - buf;
}

static uint8_t prv_dump_equws_e(const specasm_line_t *line, char *buf)
{
	return prv_dump_subtraction_e(line, buf, 0);
}

static uint8_t prv_dump_ds_e(const specasm_line_t *line, char *buf)
{
	uint16_t *count;
	char *start = buf;
	const uint8_t *op_code = line->data.op_code;

	count = (uint16_t *)&op_code[1];
	buf += specasm_dump_word(buf, *count, specasm_line_get_format2(line));

	buf[0] = ',';
	buf[1] = ' ';
	buf += 2;

	buf += specasm_dump_byte(buf, op_code[0], specasm_line_get_format(line));
	return buf - start;
}

static uint8_t prv_dump_org_e(const specasm_line_t *line, char *buf)
{
	uint16_t addr = *((uint16_t *)&line->data.op_code[0]);
	return specasm_dump_word(buf, addr, specasm_line_get_format(line));
}

static uint8_t prv_dump_align_e(const specasm_line_t *line, char *buf)
{
	return specasm_dump_word(buf, 1 << line->data.op_code[0],
			     specasm_line_get_format(line));
}

/* clang-format off */

static specasm_line_opcode_dump_t dump_opcodes[] = {
	{ prv_dump_adc_e },           /* SPECASM_LINE_TYPE_ADC */
	{ prv_dump_add_e },           /* SPECASM_LINE_TYPE_ADD */
	{ prv_dump_and_e },           /* SPECASM_LINE_TYPE_AND */
	{ prv_dump_bit_e },           /* SPECASM_LINE_TYPE_BIT */
	{ prv_dump_call_e },          /* SPECASM_LINE_TYPE_CALL */
	{ NULL },                     /* SPECASM_LINE_TYPE_CCF */
	{ prv_dump_cp_e  },           /* SPECASM_LINE_TYPE_CP */
	{ NULL },                     /* SPECASM_LINE_TYPE_CPD */
	{ NULL },                     /* SPECASM_LINE_TYPE_CPDR */
	{ NULL },                     /* SPECASM_LINE_TYPE_CPI */
	{ NULL },                     /* SPECASM_LINE_TYPE_CPIR */
	{ NULL },                     /* SPECASM_LINE_TYPE_CPL */
	{ NULL },                     /* SPECASM_LINE_TYPE_DAA */
	{ prv_dump_dec_e },           /* SPECASM_LINE_TYPE_DEC */
	{ NULL },                     /* SPECASM_LINE_TYPE_DI */
	{ prv_dump_djnz_e },          /* SPECASM_LINE_TYPE_DJNZ */
	{ NULL },                     /* SPECASM_LINE_TYPE_EI */
	{ prv_dump_ex_e },            /* SPECASM_LINE_TYPE_EX */
	{ NULL },                     /* SPECASM_LINE_TYPE_EXX */
	{ NULL },                     /* SPECASM_LINE_TYPE_HALT */
	{ prv_dump_im_e },            /* SPECASM_LINE_TYPE_IM */
	{ prv_dump_in_e },            /* SPECASM_LINE_TYPE_IN */
	{ prv_dump_inc_e },           /* SPECASM_LINE_TYPE_INC */
	{ NULL },                     /* SPECASM_LINE_TYPE_IND */
	{ NULL },                     /* SPECASM_LINE_TYPE_INDR */
	{ NULL },                     /* SPECASM_LINE_TYPE_INI */
	{ NULL },                     /* SPECASM_LINE_TYPE_INIR */
	{ prv_dump_jp_e },            /* SPECASM_LINE_TYPE_JP */
	{ prv_dump_jr_e },            /* SPECASM_LINE_TYPE_JR */
	{ prv_dump_ld_e },            /* SPECASM_LINE_TYPE_LD */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDD */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDDR */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDI */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDIR */
	{ NULL },                     /* SPECASM_LINE_TYPE_NEG */
	{ NULL },                     /* SPECASM_LINE_TYPE_NOP */
	{ prv_dump_or_e },            /* SPECASM_LINE_TYPE_OR */
	{ NULL },                     /* SPECASM_LINE_TYPE_OTDR */
	{ NULL },                     /* SPECASM_LINE_TYPE_OTIR */
	{ prv_dump_out_e },           /* SPECASM_LINE_TYPE_OUT */
	{ NULL },                     /* SPECASM_LINE_TYPE_OUTD */
	{ NULL },                     /* SPECASM_LINE_TYPE_OUTI */
	{ prv_dump_stack },           /* SPECASM_LINE_TYPE_POP */
	{ prv_dump_stack },           /* SPECASM_LINE_TYPE_PUSH */
	{ prv_dump_bit_e },           /* SPECASM_LINE_TYPE_RES */
	{ prv_dump_ret_e },           /* SPECASM_LINE_TYPE_RET */
	{ NULL },                     /* SPECASM_LINE_TYPE_RETI */
	{ NULL },                     /* SPECASM_LINE_TYPE_RETN */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_RL */
	{ NULL },                     /* SPECASM_LINE_TYPE_RLA */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_RLC */
	{ NULL },                     /* SPECASM_LINE_TYPE_RLCA */
	{ NULL },                     /* SPECASM_LINE_TYPE_RLD */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_RR */
	{ NULL },                     /* SPECASM_LINE_TYPE_RRA */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_RRC */
	{ NULL },                     /* SPECASM_LINE_TYPE_RRCA */
	{ NULL },                     /* SPECASM_LINE_TYPE_RRD */
	{ prv_dump_rst_e },           /* SPECASM_LINE_TYPE_RST */
	{ prv_dump_sbc_e },           /* SPECASM_LINE_TYPE_SBC */
	{ NULL },                     /* SPECASM_LINE_TYPE_SCF */
	{ prv_dump_bit_e },           /* SPECASM_LINE_TYPE_SET */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_SLA */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_SRA */
	{ prv_dump_shift_e },         /* SPECASM_LINE_TYPE_SRL */
	{ prv_dump_sub_e },           /* SPECASM_LINE_TYPE_SUB */
	{ prv_dump_xor_e },           /* SPECASM_LINE_TYPE_XOR */
	{ prv_dump_db_e },            /* SPECASM_LINE_TYPE_DB */
	{ prv_dump_equw_e },          /* SPECASM_LINE_TYPE_DW */
	{ prv_dump_equws_e },         /* SPECASM_LINE_TYPE_DB_SUB */
	{ prv_dump_equws_e },         /* SPECASM_LINE_TYPE_DW_SUB */
	{ prv_dump_ld_imm_16_sub_e }, /* SPECASM_LINE_TYPE_LD_IMM_16_SUB */
	{ prv_dump_ld_imm_8_sub_e },  /* SPECASM_LINE_TYPE_LD_IMM_8_SUB */
#ifdef SPECASM_TARGET_NEXT
	{ NULL },                     /* SPECASM_LINE_TYPE_LDDRX */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDDX */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDIRX */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDIX */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDPIRX */
	{ NULL },                     /* SPECASM_LINE_TYPE_LDWS */
	{ prv_dump_barrel_e },        /* SPECASM_LINE_TYPE_BRLC */
	{ prv_dump_barrel_e },        /* SPECASM_LINE_TYPE_BSLA */
	{ prv_dump_barrel_e },        /* SPECASM_LINE_TYPE_BSRA */
	{ prv_dump_barrel_e },        /* SPECASM_LINE_TYPE_BSRF */
	{ prv_dump_barrel_e },        /* SPECASM_LINE_TYPE_BSRL */
	{ NULL },                     /* SPECASM_LINE_TYPE_OUTINB */
	{ NULL },                     /* SPECASM_LINE_TYPE_SWAPNIB */
	{ NULL },                     /* SPECASM_LINE_TYPE_PIXELAD */
	{ NULL },                     /* SPECASM_LINE_TYPE_PIXELDN */
	{ NULL },                     /* SPECASM_LINE_TYPE_SETAE */
	{ prv_dump_test_e },          /* SPECASM_LINE_TYPE_TEST */
	{ prv_dump_mirror_e },        /* SPECASM_LINE_TYPE_MIRROR */
	{ prv_dump_mul_e },           /* SPECASM_LINE_TYPE_MUL */
	{ prv_dump_nextreg_e },       /* SPECASM_LINE_TYPE_NEXTREG */
#endif
	{ prv_dump_ds_e },            /* SPECASM_LINE_TYPE_REPB */
	{ prv_dump_org_e },           /* SPECASM_LINE_TYPE_ORG */
	{ NULL },                     /* SPECASM_LINE_TYPE_MAP */
	{ prv_dump_align_e },         /* SPECASM_LINE_TYPE_ALIGN */
};

/* clang-format on */

#ifdef SPECASM_NEXT_BANKED
void specasm_init_dump_table_banked(void)
#else
void specasm_init_dump_table(void)
#endif
{
	uint8_t line_type;

	for (uint8_t i = 0; i < mnemomics_table_size; i++) {
		line_type = mnemomics_table[i].line_type;
		dump_opcodes[line_type].index = i;
		if (line_type == SPECASM_LINE_TYPE_LD) {
			dump_opcodes[SPECASM_LINE_TYPE_LD_IMM_16_SUB].index = i;
			dump_opcodes[SPECASM_LINE_TYPE_LD_IMM_8_SUB].index = i;
		} else if (line_type == SPECASM_LINE_TYPE_DB) {
			dump_opcodes[SPECASM_LINE_TYPE_DB_SUB].index = i;
		} else if (line_type == SPECASM_LINE_TYPE_DW) {
			dump_opcodes[SPECASM_LINE_TYPE_DW_SUB].index = i;
		}
	}
}

#ifdef SPECASM_NEXT_BANKED
uint8_t specasm_dump_opcode_banked_e(const specasm_line_t *line, char *buf)
#else
uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf)
#endif
{
	uint8_t index;
	const char *name;
	char *start = buf;
	uint8_t type = specasm_line_get_adj_type(line);

	index = dump_opcodes[type].index;
	name = mnemomics_table[index].mnemomic;
	while (*name)
		*buf++ = *name++;
	if (dump_opcodes[type].fn) {
		if (type != SPECASM_LINE_TYPE_RET)
			*buf++ = ' ';
		buf += dump_opcodes[type].fn(line, buf);
	}
	return buf - start;
}
