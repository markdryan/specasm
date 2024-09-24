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

#include <stdint.h>
#include <stdlib.h>

#include "analysis.h"

typedef uint8_t (*specasm_analysis_fn_t)(const specasm_line_t *line,
					 specasm_cycles_t *cycles);

struct specasm_meta_entry_t_ {
	specasm_analysis_fn_t fn;
	specasm_cycles_t cycles;
	uint8_t flags;
};

typedef struct specasm_meta_entry_t_ specasm_meta_entry_t;

uint8_t prv_anal_adc_sbc(const specasm_line_t *line,specasm_cycles_t *cycles,
			 uint8_t hl_ind, uint8_t imm)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == imm) || (op_code[0] == hl_ind)) {
		/*
		 * adc a, n
		 * adc a, (hl)
		 */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * adc a, (ix+d)
		 * adc a, (iy+d)
		 */
		t1 = 19;
		t2 = 19;
		m1 = 5;
		m2 = 5;
	} else if (op_code[0] == 0xed) {
		/* adc hl, ss */
		t1 = 15;
		t2 = 15;
		m1 = 4;
		m2 = 4;
	} else {
		/* adc a, r */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0xd7; // szhpnc
}


uint8_t prv_anal_adc(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_adc_sbc(line, cycles, 0x8e, 0xce);
}

uint8_t prv_anal_add(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;
	uint8_t flags = 0xd7; // szhpnc

	if (op_code[0] == 0x86) {
		/* add a, (hl) */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		if (specasm_line_get_size(line) > 1) {
			/*
			 * add a, (ix+d)
			 * add a, (iy+d)
			 */
			t1 = 19;
			t2 = 19;
			m1 = 5;
			m2 = 5;
		} else {
			/*
			 * add ix, ix
			 * add iy, iy
			 */
			t1 = 15;
			t2 = 15;
			m1 = 4;
			m2 = 4;
			flags = 0x13;
		}
	} else if (op_code[0] == 0xc6) {
		/* add a, n */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] & 0xcf) == 9) {
		/* add hl, ss */
		t1 = 11;
		t2 = 11;
		m1 = 3;
		m2 = 3;
		flags = 0x13;
#ifdef SPECASM_TARGET_NEXT_OPCODES
	} else if (op_code[0] == 0xED) {
		if (specasm_line_get_size(line) > 1) {
			/*
			 * add bc, imm16
			 * add de, imm16
			 * add hl, imm16
			 */
			t1 = 16;
			t2 = 16;
			m1 = 4;
			m2 = 4;
			flags = 0;
		} else {
			t1 = 8;
			t2 = 8;
			m1 = 2;
			m2 = 2;
			flags = 1; /* c is undefined apparently */
		}
#endif
	} else {
		/* add a, r */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return flags;
}

uint8_t prv_anal_logical(const specasm_line_t *line,specasm_cycles_t *cycles,
			 uint8_t hl_ind, uint8_t imm)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == imm) || (op_code[0] == hl_ind)) {
		/*
		 *and imm8
		* and (hl)
		*/
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * and (ix+d)
		 * and (iy+d)
		 */
		t1 = 19;
		t2 = 19;
		m1 = 5;
		m2 = 5;
	} else {
		/*
		 * and r
		 */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0xd7;  // szhpnc
}

uint8_t prv_anal_and(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_logical(line, cycles, 0xa6, 0xe6);
}

uint8_t prv_anal_bit(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == 0xcb) && ((op_code[1] & 0xc7)== 0x46)) {
		/* bit b, (hl) */
		t1 = 12;
		t2 = 12;
		m1 = 3;
		m2 = 3;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * bit b, (ix+d)
		 * bit b, (iy+d)
		 */
		t1 = 20;
		t2 = 20;
		m1 = 5;
		m2 = 5;
	} else {
		/* bit b, r */
		t1 = 8;
		t2 = 8;
		m1 = 2;
		m2 = 2;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0xd6; // szhpn
}

uint8_t prv_anal_call(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if (op_code[0] == 0xcd) {
		/* call label */
		t1 = 17;
		t2 = 17;
		m1 = 5;
		m2 = 5;
	} else {
		/* call cc, label */
		t1 = 10;
		t2 = 17;
		m1 = 3;
		m2 = 5;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_cp_e(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == 0xfe) || (op_code[0] == 0xbe)) {
		/*
		 * cp n
		 * cp (hl)
		 */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * cp (ix+d)
		 * cp (iy+d)
		 */
		t1 = 19;
		t2 = 19;
		m1 = 5;
		m2 = 5;
	} else {
		/* cp r */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0xd7; /* szhpnc*/
}

uint8_t prv_anal_inc_dec(const specasm_line_t *line,specasm_cycles_t *cycles,
			 uint8_t ind_hl, uint8_t rr)
{
	uint8_t t1, t2 ,m1, m2;
	const uint8_t *op_code = line->data.op_code;
	uint8_t flags = 0xd6; /* szhpn */

	if (op_code[0] == ind_hl) {
		/* dec (hl) */
		t1 = 11;
		t2 = 11;
		m1 = 3;
		m2 = 3;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		if (specasm_line_get_size(line) > 1) {
			/*
			 * dec (ix+d)
			 * dec (iy+d)
			 */
			t1 = 23;
			t2 = 23;
			m1 = 6;
			m2 = 6;
		} else {
			/*
			 * dec ix
			 * dec iy
			 */
			t1 = 10;
			t2 = 10;
			m1 = 2;
			m2 = 2;
			flags = 0;
		}
	} else if ((op_code[0] & 0xcf) == rr) {
		/* dec rr */
		t1 = 6;
		t2 = 6;
		m1 = 1;
		m2 = 1;
		flags = 0;
	} else {
		/* adc a, r */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return flags;
}

uint8_t prv_anal_dec(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_inc_dec(line, cycles, 0x35, 0xb);
}

uint8_t prv_anal_ex(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	uint8_t flags = 0;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0x8) {
		/* ex af, af' */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
		flags = 0xd7; /* szhpnc */
	} else if (op_code[0] == 0xeb) {
		/* ex de, hl */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	} else if (op_code[0] == 0xe3) {
		/* ex (sp), hl */
		t1 = 19;
		t2 = 19;
		m1 = 5;
		m2 = 5;
	} else {
		/*
		 * ex (sp), ix
		 * ex (sp), iy
		 */
		t1 = 23;
		t2 = 23;
		m1 = 6;
		m2 = 6;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return flags;
}

uint8_t prv_anal_in(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	uint8_t flags = 0;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0xed) {
		/* in r, (c) */
		t1 = 12;
		t2 = 12;
		m1 = 3;
		m2 = 3;
		flags = 0xd6; /* szhpn */
	} else {
		/* in a, (n) */
		t1 = 11;
		t2 = 11;
		m1 = 3;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return flags;
}

uint8_t prv_anal_inc(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_inc_dec(line, cycles, 0x34, 0x3);
}

uint8_t prv_anal_jp(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	uint8_t flags = 0;
	const uint8_t *op_code = line->data.op_code;

	if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * jp (ix)
		 * jp (iy)
		 */
		t1 = 8;
		t2 = 8;
		m1 = 2;
		m2 = 2;
	} else if (op_code[0] == 0xe9) {
		/* jp (hl) */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
#ifdef SPECASM_TARGET_NEXT_OPCODES
	} else if (op_code[0] == 0xed) {
		/* jp (c) */
		t1 = 13;
		t2 = 13;
		m1 = 3;
		m2 = 3;
		flags = 0xd7; /* szfpnc */
#endif
	} else {
		/*
		 * jp label
		 * jp cc, label
		 */
		t1 = 10;
		t2 = 10;
		m1 = 3;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return flags;
}

uint8_t prv_anal_jr(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	const uint8_t *op_code = line->data.op_code;

	if (op_code[0] == 0x18) {
		/* jr label */
		t1 = 12;
		t2 = 12;
		m1 = 3;
		m2 = 3;
	} else {
		/* jr cc, label */
		t1 = 7;
		t2 = 12;
		m1 = 2;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_ld(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	const uint8_t *op_code = line->data.op_code;

	if ((op_code[0] == 0xfd) || (op_code[0] == 0xdd)) {
		if (op_code[1] == 0x21) {
			/*
			 * ld ix, imm16
			 * ld iy, imm16
			 */
			t1 = 14;
			t2 = 14;
			m1 = 4;
			m2 = 4;
		} else  if ((op_code[1] == 0x2a) || (op_code[1] == 0x22)) {
			/*
			 * ld ix, (imm16)
			 * ld iy, (imm16)
			 * ld (imm16), ix
			 * ld (imm16), iy
			 */
			t1 = 20;
			t2 = 20;
			m1 = 6;
			m2 = 6;
		} else if (op_code[1] == 0xf9) {
			/*
			 * ld sp, ix
			 * ld sp, iy
			 */
			t1 = 10;
			t2 = 10;
			m1 = 2;
			m2 = 2;
		} else {
			/*
			 * ld r, (ix+d)
			 * ld r, (iy+d)
			 * ld (ix+d), r
			 * ld (iy+d), r
			 * ld (ix+d), imm8
			 * ld (iy+d), imm8
			 */
			t1 = 19;
			t2 = 19;
			m1 = 5;
			m2 = 5;
		}
	} else if (op_code[0] == 0xed) {
		if ((op_code[1] == 0x47) || (op_code[1] == 0x57) ||
		    (op_code[1] == 0x4f) || (op_code[1] == 0x5f)) {
			/*
			 * ld a, i
			 * ld a, r
			 * ld i, a
			 * ld r, a
			 */

			t1 = 9;
			t2 = 9;
			m1 = 2;
			m2 = 2;
		} else {
			/*
			 * ld rr, (imm16)
			 * ld (imm16), rr
			 */
			t1 = 20;
			t2 = 20;
			m1 = 6;
			m2 = 6;
		}
	} else if ((op_code[0] == 0x2a) || (op_code[0] == 0x22)) {
		/*
		 * ld (hl), imm16
		 * ld imm16, (hl)
		 */
		t1 = 16;
		t2 = 16;
		m1 = 5;
		m2 = 5;
	} else if ((op_code[0] == 0xed) || (op_code[0] == 0xf9)) {
		/*
		 * ld hl. sp
		 * ld sp, hl
		 */
		t1 = 6;
		t2 = 6;
		m1 = 1;
		m2 = 1;
	} else if (((op_code[0] & 0xcf) == 1) || (op_code[0] == 0x36)) {
		/*
		 * ld rr. imm16
		 * ld (hl), imm8
		 */
		t1 = 10;
		t2 = 10;
		m1 = 3;
		m2 = 3;
	} else if ((op_code[0] & 0xc7) == 0x06) {
		/*
		 * ld b, 10'
		 */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 2) || (op_code[0] == 0x12) ||
		   (op_code[0] == 0xa) || (op_code[0] == 0x1a) ||
		   ((op_code[0] & 0xf8) == 0x70) ||
		   ((op_code[0] & 0xc7) == 0x46)) {
		/*
		 * ld (bc), a
		 * ld (de), a
		 * ld a, (bc)
		 * ld a, (de)
		 * ld (hl), r
		 * ld r, (hl)
		 */
		t1 = 7;
		t2 = 7;
		m1 = 2;
		m2 = 2;
	} else if ((op_code[0] == 0x3a) || (op_code[0] == 0x32)) {
		/*
		 * ld a, (imm16)
		 * ld (imm16), a
		 */
		t1 = 13;
		t2 = 13;
		m1 = 4;
		m2 = 4;
	} else {
		/*
		 * ld r, r'
		 */
		t1 = 4;
		t2 = 4;
		m1 = 1;
		m2 = 1;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_or(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_logical(line, cycles, 0xb6, 0xf6);
}

uint8_t prv_anal_out(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	/*
	 * in and out are almost the same but out doesn't affect the flags
	 */
	(void) prv_anal_in(line, cycles);
	return 0;
}

uint8_t prv_anal_pop(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	const uint8_t *op_code = line->data.op_code;

	if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)){
		/*
		 * pop ix
		 * pop iy
		 */
		t1 = 14;
		t2 = 14;
		m1 = 4;
		m2 = 4;
	} else {
		/* pop rr */
		t1 = 10;
		t2 = 10;
		m1 = 3;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_push(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	uint8_t t1, t2 ,m1, m2;
	const uint8_t *op_code = line->data.op_code;

	if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)){
		/*
		 * push ix
		 * push iy
		 */
		t1 = 15;
		t2 = 15;
		m1 = 4;
		m2 = 4;
#ifdef SPECASM_TARGET_NEXT_OPCODES
	} else if (op_code[0] == 0xed) {
		/* push imm16 */
		t1 = 23;
		t2 = 23;
		m1 = 6;
		m2 = 6;
#endif
	} else {
		/* push rr */
		t1 = 11;
		t2 = 11;
		m1 = 3;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}
uint8_t prv_anal_bit_write(const specasm_line_t *line,specasm_cycles_t *cycles,
			   uint8_t hl_ind)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == 0xcb) && ((op_code[1] & 0xc7)== hl_ind)) {
		/* set/res b, (hl) */
		t1 = 15;
		t2 = 15;
		m1 = 4;
		m2 = 4;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * set/res b, (ix+d)
		 * set/res b, (iy+d)
		 */
		t1 = 23;
		t2 = 23;
		m1 = 6;
		m2 = 6;
	} else {
		/* set/res b, r */
		t1 = 8;
		t2 = 8;
		m1 = 2;
		m2 = 2;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_res(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_bit_write(line, cycles, 0x86);
}

uint8_t prv_anal_ret(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if (op_code[0] == 0xc9) {
		/* ret */
		t1 = 10;
		t2 = 10;
		m1 = 3;
		m2 = 3;
	} else {
		/* ret cc */
		t1 = 5;
		t2 = 11;
		m1 = 1;
		m2 = 3;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}

uint8_t prv_anal_set(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_bit_write(line, cycles, 0xc6);
}

uint8_t prv_anal_shift(const specasm_line_t *line,specasm_cycles_t *cycles,
		       uint8_t hl_ind)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if ((op_code[0] == 0xcb) && (op_code[1] == hl_ind)) {
		/*
		 * rl (hl)
		 * rr (hl)
		 * rlc (hl)
		 */
		t1 = 15;
		t2 = 15;
		m1 = 4;
		m2 = 4;
	} else if ((op_code[0] == 0xdd) || (op_code[0] == 0xfd)) {
		/*
		 * rl (ix+d)
		 * rl (iy+d)
		 * rr (ix+d)
		 * rr (iy+d)
		 * rlc (ix+d)
		 * rlc (iy+d)
		 */
		t1 = 23;
		t2 = 23;
		m1 = 6;
		m2 = 6;
	} else {
		/*
		 * rl r
		 * rr r
		 * rlc r
		 */
		t1 = 8;
		t2 = 8;
		m1 = 2;
		m2 = 2;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0xd7;  // szhpnc
}

uint8_t prv_anal_rl(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x16);
}

uint8_t prv_anal_rr(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x1e);
}

uint8_t prv_anal_rlc(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x6);
}

uint8_t prv_anal_rrc(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x0e);
}

uint8_t prv_anal_sbc(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_adc_sbc(line, cycles, 0x9e, 0xde);
}

uint8_t prv_anal_sla(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x26);
}

uint8_t prv_anal_sra(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x2e);
}

uint8_t prv_anal_srl(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_shift(line, cycles, 0x3e);
}

uint8_t prv_anal_sub(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_adc_sbc(line, cycles, 0x96, 0xd6);
}

uint8_t prv_anal_xor(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	return prv_anal_logical(line, cycles, 0xae, 0xee);
}

#ifdef SPECASM_TARGET_NEXT_OPCODES
uint8_t prv_anal_nextreg(const specasm_line_t *line,specasm_cycles_t *cycles)
{
	const uint8_t *op_code = line->data.op_code;
	uint8_t t1, t2 ,m1, m2;

	if (op_code[1] == 0x92) {
		/* nextreg n, A */
		t1 = 17;
		t2 = 17;
		m1 = 4;
		m2 = 4;
	} else {
		/* nextreg n, m */
		t1 = 20;
		t2 = 20;
		m1 = 5;
		m2 = 5;
	}

	cycles->t[0] = t1;
	cycles->t[1] = t2;
	cycles->m[0] = m1;
	cycles->m[1] = m2;

	return 0;
}
#endif

/* clang-format off */

static specasm_meta_entry_t anal_opcodes[] = {
	{ prv_anal_adc },                   /* SPECASM_LINE_TYPE_ADC */
	{ prv_anal_add },                   /* SPECASM_LINE_TYPE_ADD */
	{ prv_anal_and },                   /* SPECASM_LINE_TYPE_AND */
	{ prv_anal_bit },                   /* SPECASM_LINE_TYPE_BIT */
	{ prv_anal_call },                  /* SPECASM_LINE_TYPE_CALL */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_CCF */
	{ prv_anal_cp_e  },                 /* SPECASM_LINE_TYPE_CP */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_CPD */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_CPDR */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_CPI */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_CPIR */
	{ NULL, {{1, 1}, {4, 4}}, 0x12 },   /* SPECASM_LINE_TYPE_CPL */
	{ NULL, {{1, 1}, {4, 4}}, 0xd5 },   /* SPECASM_LINE_TYPE_DAA */
	{ prv_anal_dec },                   /* SPECASM_LINE_TYPE_DEC */
	{ NULL, {{1, 1}, {4, 4}}, 0 },      /* SPECASM_LINE_TYPE_DI */
	{ NULL, {{2, 3}, {8, 13}}, 0 },     /* SPECASM_LINE_TYPE_DJNZ */
	{ NULL, {{1, 1}, {4, 4}}, 0 },      /* SPECASM_LINE_TYPE_EI */
	{ prv_anal_ex },                    /* SPECASM_LINE_TYPE_EX */
	{ NULL, {{1, 1}, {4, 4}}, 0 },      /* SPECASM_LINE_TYPE_EXX */
	{ NULL, {{1, 1}, {4, 4}}, 0 },      /* SPECASM_LINE_TYPE_HALT */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_IM */
	{ prv_anal_in },                    /* SPECASM_LINE_TYPE_IN */
	{ prv_anal_inc },                   /* SPECASM_LINE_TYPE_INC */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_IND */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_INDR */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_INI */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_INIR */
	{ prv_anal_jp },                    /* SPECASM_LINE_TYPE_JP */
	{ prv_anal_jr },                    /* SPECASM_LINE_TYPE_JR */
	{ prv_anal_ld },                    /* SPECASM_LINE_TYPE_LD */
	{ NULL, {{4, 4}, {16, 16}}, 0x16 }, /* SPECASM_LINE_TYPE_LDD */
	{ NULL, {{4, 5}, {16, 21}}, 0x16 }, /* SPECASM_LINE_TYPE_LDDR */
	{ NULL, {{4, 4}, {16, 16}}, 0x16 }, /* SPECASM_LINE_TYPE_LDI */
	{ NULL, {{4, 5}, {16, 21}}, 0x16 }, /* SPECASM_LINE_TYPE_LDIR */
	{ NULL, {{2, 2}, {8, 8 }}, 0xd7 },  /* SPECASM_LINE_TYPE_NEG */
	{ NULL, {{1, 1}, {4, 4}}, 0 },      /* SPECASM_LINE_TYPE_NOP */
	{ prv_anal_or },                    /* SPECASM_LINE_TYPE_OR */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_OTDR */
	{ NULL, {{4, 5}, {16, 21}}, 0xd6 }, /* SPECASM_LINE_TYPE_OTIR */
	{ prv_anal_out },                   /* SPECASM_LINE_TYPE_OUT */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_OUTD */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_OUTI */
	{ prv_anal_pop },                   /* SPECASM_LINE_TYPE_POP */
	{ prv_anal_push },                  /* SPECASM_LINE_TYPE_PUSH */
	{ prv_anal_res },                   /* SPECASM_LINE_TYPE_RES */
	{ prv_anal_ret },                   /* SPECASM_LINE_TYPE_RET */
	{ NULL, {{4, 4}, {14, 14}}, 0 },    /* SPECASM_LINE_TYPE_RETI */
	{ NULL, {{4, 4}, {14, 14}}, 0 },    /* SPECASM_LINE_TYPE_RETN */
	{ prv_anal_rl },                    /* SPECASM_LINE_TYPE_RL */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_RLA */
	{ prv_anal_rlc },                   /* SPECASM_LINE_TYPE_RLC */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_RLCA */
	{ NULL, {{5, 5}, {18, 18}}, 0xd6 }, /* SPECASM_LINE_TYPE_RLD */
	{ prv_anal_rr },                    /* SPECASM_LINE_TYPE_RR */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_RRA */
	{ prv_anal_rrc },                   /* SPECASM_LINE_TYPE_RRC */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_RRCA */
	{ NULL, {{5, 5}, {18, 18}}, 0xd6 }, /* SPECASM_LINE_TYPE_RRD */
	{ NULL, {{3, 3}, {11, 11}}, 0 },    /* SPECASM_LINE_TYPE_RST */
	{ prv_anal_sbc },                   /* SPECASM_LINE_TYPE_SBC */
	{ NULL, {{1, 1}, {4, 4}}, 0x13 },   /* SPECASM_LINE_TYPE_SCF */
	{ prv_anal_set },                   /* SPECASM_LINE_TYPE_SET */
	{ prv_anal_sla },                   /* SPECASM_LINE_TYPE_SLA */
	{ prv_anal_sra },                   /* SPECASM_LINE_TYPE_SRA */
	{ prv_anal_srl },                   /* SPECASM_LINE_TYPE_SRL */
	{ prv_anal_sub },                   /* SPECASM_LINE_TYPE_SUB */
	{ prv_anal_xor },                   /* SPECASM_LINE_TYPE_XOR */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_DB */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_DW */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_DB_SUB */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_DW_SUB */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_LD_IMM_16_SUB */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_LD_IMM_8_SUB */
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ NULL },                           /* Spare */
	{ NULL },                           /* Spare */
	{ NULL, {{4, 5}, {16, 21}}, 0 },    /* SPECASM_LINE_TYPE_LDDRX */
	{ NULL, {{4, 4}, {16, 16}}, 0 },    /* SPECASM_LINE_TYPE_LDDX */
	{ NULL, {{4, 5}, {16, 21}}, 0 },    /* SPECASM_LINE_TYPE_LDIRX */
	{ NULL, {{4, 4}, {16, 16}}, 0 },    /* SPECASM_LINE_TYPE_LDIX */
	{ NULL, {{4, 5}, {16, 21}}, 0 },    /* SPECASM_LINE_TYPE_LDPIRX */
	{ NULL, {{4, 4}, {16, 16}}, 0xd6 }, /* SPECASM_LINE_TYPE_LDWS */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_BRLC */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_BSLA */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_BSRA */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_BSRF */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_BSRL */
	{ NULL, {{4, 4}, {16, 16}}, 0xd7 }, /* SPECASM_LINE_TYPE_OUTINB */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_SWAPNIB */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_PIXELAD */
	{ NULL, {{2, 2}, {8, 8}}, 0},       /* SPECASM_LINE_TYPE_PIXELDN */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_SETAE */
	{ NULL, {{3, 3}, {11, 11}}, 0xd7 }, /* SPECASM_LINE_TYPE_TEST */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_MIRROR */
	{ NULL, {{2, 2}, {8, 8}}, 0 },      /* SPECASM_LINE_TYPE_MUL */
	{ prv_anal_nextreg },               /* SPECASM_LINE_TYPE_NEXTREG */
	{ NULL, {{5, 5}, {20, 20}}, 0 },    /* SPECASM_LINE_TYPE_NBRK */
#endif
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_REPB */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_ORG */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_MAP */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_ALIGN */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_EQU */
	{ NULL, {{0, 0}, {0, 0}}, 0 },      /* SPECASM_LINE_TYPE_ZX81 */
};

/* clang-format on */

void specasm_get_cycles(const specasm_line_t *line, specasm_cycles_t* cycles)
{
	const specasm_meta_entry_t *meta;
	uint8_t type = specasm_line_get_adj_type(line);

	if (type > SPECASM_LINE_TYPE_SIMPLE_MAX) {
		cycles->m[0] = 0;
		cycles->m[1] = 0;
		cycles->t[0] = 0;
		cycles->t[1] = 0;
		return;
	}

	meta = &anal_opcodes[type];

	if (meta->fn) {
		(void) meta->fn(line, cycles);
		return;
	}

	cycles->m[0] = meta->cycles.m[0];
	cycles->m[1] = meta->cycles.m[1];
	cycles->t[0] = meta->cycles.t[0];
	cycles->t[1] = meta->cycles.t[1];
}

uint8_t specasm_get_flags(const specasm_line_t *line)
{
	specasm_cycles_t cycles;
	const specasm_meta_entry_t *meta;
	uint8_t type = specasm_line_get_adj_type(line);

	if (type > SPECASM_LINE_TYPE_SIMPLE_MAX)
		return 0;

	meta = &anal_opcodes[type];

	if (meta->fn)
		return meta->fn(line, &cycles);

	return meta->flags;
}
