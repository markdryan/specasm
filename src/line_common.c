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

#include "line_common.h"

/* clang-format off */

char byte_regs[8] = {'b', 'c', 'd', 'e', 'h', 'l', ' ', 'a'};

/*
 * Must be in the same order as the opcode_table in line_parse.c.
 */

const specasm_mnemomic_t mnemomics_table[] = {
	{ "adc", SPECASM_LINE_TYPE_ADC, },
	{ "add", SPECASM_LINE_TYPE_ADD, },
	{ "align", SPECASM_LINE_TYPE_ALIGN, },
	{ "and", SPECASM_LINE_TYPE_AND, },
	{ "bit", SPECASM_LINE_TYPE_BIT, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "brlc", SPECASM_LINE_TYPE_BRLC, },
	{ "bsla", SPECASM_LINE_TYPE_BSLA, },
	{ "bsra", SPECASM_LINE_TYPE_BSRA, },
	{ "bsrf", SPECASM_LINE_TYPE_BSRF, },
	{ "bsrl", SPECASM_LINE_TYPE_BSRL, },
#endif
	{ "call", SPECASM_LINE_TYPE_CALL, },
	{ "ccf", SPECASM_LINE_TYPE_CCF, },
	{ "cp", SPECASM_LINE_TYPE_CP, },
	{ "cpd", SPECASM_LINE_TYPE_CPD, },
	{ "cpdr", SPECASM_LINE_TYPE_CPDR },
	{ "cpi", SPECASM_LINE_TYPE_CPI, },
	{ "cpir", SPECASM_LINE_TYPE_CPIR, },
	{ "cpl", SPECASM_LINE_TYPE_CPL, },
	{ "daa", SPECASM_LINE_TYPE_DAA, },
	{ "db", SPECASM_LINE_TYPE_DB, },
	{ "dec", SPECASM_LINE_TYPE_DEC, },
	{ "di", SPECASM_LINE_TYPE_DI, },
	{ "djnz", SPECASM_LINE_TYPE_DJNZ, },
	{ "ds", SPECASM_LINE_TYPE_DS, },
	{ "dw", SPECASM_LINE_TYPE_DW, },
	{ "ei", SPECASM_LINE_TYPE_EI, },
	{ "ex", SPECASM_LINE_TYPE_EX, },
	{ "exx", SPECASM_LINE_TYPE_EXX, },
	{ "halt", SPECASM_LINE_TYPE_HALT, },
	{ "im", SPECASM_LINE_TYPE_IM, },
	{ "in", SPECASM_LINE_TYPE_IN },
	{ "inc", SPECASM_LINE_TYPE_INC, },
	{ "ind", SPECASM_LINE_TYPE_IND, },
	{ "indr", SPECASM_LINE_TYPE_INDR, },
	{ "ini", SPECASM_LINE_TYPE_INI, },
	{ "inir", SPECASM_LINE_TYPE_INIR, },
	{ "jp", SPECASM_LINE_TYPE_JP, },
	{ "jr", SPECASM_LINE_TYPE_JR, },
	{ "ld", SPECASM_LINE_TYPE_LD, },
	{ "ldd", SPECASM_LINE_TYPE_LDD, },
	{ "lddr", SPECASM_LINE_TYPE_LDDR, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "lddrx", SPECASM_LINE_TYPE_LDDRX, },
	{ "lddx", SPECASM_LINE_TYPE_LDDX, },
#endif
	{ "ldi", SPECASM_LINE_TYPE_LDI, },
	{ "ldir", SPECASM_LINE_TYPE_LDIR, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "ldirx", SPECASM_LINE_TYPE_LDIRX, },
	{ "ldix", SPECASM_LINE_TYPE_LDIX, },
	{ "ldpirx", SPECASM_LINE_TYPE_LDPIRX, },
	{ "ldws", SPECASM_LINE_TYPE_LDWS, },
#endif
	{ "map", SPECASM_LINE_TYPE_MAP, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "mirror", SPECASM_LINE_TYPE_MIRROR, },
	{ "mul", SPECASM_LINE_TYPE_MUL, },
	{ "nbrk", SPECASM_LINE_TYPE_NBRK, },
#endif
	{ "neg", SPECASM_LINE_TYPE_NEG, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "nextreg", SPECASM_LINE_TYPE_NEXTREG, },
#endif
	{ "nop", SPECASM_LINE_TYPE_NOP, },
	{ "or", SPECASM_LINE_TYPE_OR, },
	{ "org", SPECASM_LINE_TYPE_ORG, },
	{ "otdr", SPECASM_LINE_TYPE_OTDR, },
	{ "otir", SPECASM_LINE_TYPE_OTIR, },
	{ "out", SPECASM_LINE_TYPE_OUT, },
	{ "outd", SPECASM_LINE_TYPE_OUTD, },
	{ "outi", SPECASM_LINE_TYPE_OUTI, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "outinb", SPECASM_LINE_TYPE_OUTINB, },
	{ "pixelad", SPECASM_LINE_TYPE_PIXELAD, },
	{ "pixeldn", SPECASM_LINE_TYPE_PIXELDN, },
#endif
	{ "pop", SPECASM_LINE_TYPE_POP, },
	{ "push", SPECASM_LINE_TYPE_PUSH, },
	{ "res", SPECASM_LINE_TYPE_RES, },
	{ "ret", SPECASM_LINE_TYPE_RET, },
	{ "reti", SPECASM_LINE_TYPE_RETI, },
	{ "retn", SPECASM_LINE_TYPE_RETN, },
	{ "rl", SPECASM_LINE_TYPE_RL, },
	{ "rla", SPECASM_LINE_TYPE_RLA, },
	{ "rlc", SPECASM_LINE_TYPE_RLC, },
	{ "rlca", SPECASM_LINE_TYPE_RLCA, },
	{ "rld", SPECASM_LINE_TYPE_RLD, },
	{ "rr", SPECASM_LINE_TYPE_RR, },
	{ "rra", SPECASM_LINE_TYPE_RRA, },
	{ "rrc", SPECASM_LINE_TYPE_RRC, },
	{ "rrca", SPECASM_LINE_TYPE_RRCA, },
	{ "rrd", SPECASM_LINE_TYPE_RRD, },
	{ "rst", SPECASM_LINE_TYPE_RST, },
	{ "sbc", SPECASM_LINE_TYPE_SBC, },
	{ "scf", SPECASM_LINE_TYPE_SCF, },
	{ "set", SPECASM_LINE_TYPE_SET, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "setae", SPECASM_LINE_TYPE_SETAE, },
#endif
	{ "sla", SPECASM_LINE_TYPE_SLA, },
	{ "sra", SPECASM_LINE_TYPE_SRA, },
	{ "srl", SPECASM_LINE_TYPE_SRL, },
	{ "sub", SPECASM_LINE_TYPE_SUB, },
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{ "swapnib", SPECASM_LINE_TYPE_SWAPNIB, },
	{ "test", SPECASM_LINE_TYPE_TEST, },
#endif
	{ "xor", SPECASM_LINE_TYPE_XOR, },
	{ "zx81", SPECASM_LINE_TYPE_ZX81, },
};

/* clang-format on */

const uint8_t mnemomics_table_size =
    sizeof(mnemomics_table) / sizeof(const specasm_mnemomic_t);
