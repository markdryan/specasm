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

#ifndef SPECASM_LINE_H
#define SPECASM_LINE_H

#include "error.h"

#define SPECASM_LINE_TYPE_ADC 0
#define SPECASM_LINE_TYPE_ADD 1
#define SPECASM_LINE_TYPE_AND 2
#define SPECASM_LINE_TYPE_BIT 3
#define SPECASM_LINE_TYPE_CALL 4
#define SPECASM_LINE_TYPE_CCF 5
#define SPECASM_LINE_TYPE_CP 6
#define SPECASM_LINE_TYPE_CPD 7
#define SPECASM_LINE_TYPE_CPDR 8
#define SPECASM_LINE_TYPE_CPI 9
#define SPECASM_LINE_TYPE_CPIR 10
#define SPECASM_LINE_TYPE_CPL 11
#define SPECASM_LINE_TYPE_DAA 12
#define SPECASM_LINE_TYPE_DEC 13
#define SPECASM_LINE_TYPE_DI 14
#define SPECASM_LINE_TYPE_DJNZ 15
#define SPECASM_LINE_TYPE_EI 16
#define SPECASM_LINE_TYPE_EX 17
#define SPECASM_LINE_TYPE_EXX 18
#define SPECASM_LINE_TYPE_HALT 19
#define SPECASM_LINE_TYPE_IM 20
#define SPECASM_LINE_TYPE_IN 21
#define SPECASM_LINE_TYPE_INC 22
#define SPECASM_LINE_TYPE_IND 23
#define SPECASM_LINE_TYPE_INDR 24
#define SPECASM_LINE_TYPE_INI 25
#define SPECASM_LINE_TYPE_INIR 26
#define SPECASM_LINE_TYPE_JP 27
#define SPECASM_LINE_TYPE_JR 28
#define SPECASM_LINE_TYPE_LD 29
#define SPECASM_LINE_TYPE_LDD 30
#define SPECASM_LINE_TYPE_LDDR 31
#define SPECASM_LINE_TYPE_LDI 32
#define SPECASM_LINE_TYPE_LDIR 33
#define SPECASM_LINE_TYPE_NEG 34
#define SPECASM_LINE_TYPE_NOP 35
#define SPECASM_LINE_TYPE_OR 36
#define SPECASM_LINE_TYPE_OTDR 37
#define SPECASM_LINE_TYPE_OTIR 38
#define SPECASM_LINE_TYPE_OUT 39
#define SPECASM_LINE_TYPE_OUTD 40
#define SPECASM_LINE_TYPE_OUTI 41
#define SPECASM_LINE_TYPE_POP 42
#define SPECASM_LINE_TYPE_PUSH 43
#define SPECASM_LINE_TYPE_RES 44
#define SPECASM_LINE_TYPE_RET 45
#define SPECASM_LINE_TYPE_RETI 46
#define SPECASM_LINE_TYPE_RETN 47
#define SPECASM_LINE_TYPE_RL 48
#define SPECASM_LINE_TYPE_RLA 49
#define SPECASM_LINE_TYPE_RLC 50
#define SPECASM_LINE_TYPE_RLCA 51
#define SPECASM_LINE_TYPE_RLD 52
#define SPECASM_LINE_TYPE_RR 53
#define SPECASM_LINE_TYPE_RRA 54
#define SPECASM_LINE_TYPE_RRC 55
#define SPECASM_LINE_TYPE_RRCA 56
#define SPECASM_LINE_TYPE_RRD 57
#define SPECASM_LINE_TYPE_RST 58
#define SPECASM_LINE_TYPE_SBC 59
#define SPECASM_LINE_TYPE_SCF 60
#define SPECASM_LINE_TYPE_SET 61
#define SPECASM_LINE_TYPE_SLA 62
#define SPECASM_LINE_TYPE_SRA 63
#define SPECASM_LINE_TYPE_SRL 64
#define SPECASM_LINE_TYPE_SUB 65
#define SPECASM_LINE_TYPE_XOR 66
#define SPECASM_LINE_TYPE_DB 67
#define SPECASM_LINE_TYPE_DW 68
#define SPECASM_LINE_TYPE_DB_SUB 69
#define SPECASM_LINE_TYPE_DW_SUB 70
#define SPECASM_LINE_TYPE_LD_IMM_16_SUB 71
#define SPECASM_LINE_TYPE_LD_IMM_8_SUB 72
#define SPECASM_LINE_TYPE_SIMPLE_MAX SPECASM_LINE_TYPE_LD_IMM_8_SUB
#define SPECASM_LINE_TYPE_DS (SPECASM_LINE_TYPE_SIMPLE_MAX + 1)
#define SPECASM_LINE_TYPE_ORG (SPECASM_LINE_TYPE_SIMPLE_MAX + 2)
#define SPECASM_LINE_TYPE_MAP (SPECASM_LINE_TYPE_SIMPLE_MAX + 3)
#define SPECASM_LINE_TYPE_ALIGN (SPECASM_LINE_TYPE_SIMPLE_MAX + 4)
#define SPECASM_LINE_TYPE_MAX (SPECASM_LINE_TYPE_ALIGN + 1)

#define SPECASM_LINE_TYPE_EMPTY 128
#define SPECASM_LINE_TYPE_LL 129
#define SPECASM_LINE_TYPE_SL 130
#define SPECASM_LINE_TYPE_LC 131
#define SPECASM_LINE_TYPE_SC 132

// These need to stay sequential and the short types
// need to be even, the longs odd.  The LINE_TYPE
// need to be sequential as well.
#define SPECASM_LINE_TYPE_STR_SIN_SHORT 134
#define SPECASM_LINE_TYPE_STR_SIN_LONG 135
#define SPECASM_LINE_TYPE_STR_DBL_SHORT 136
#define SPECASM_LINE_TYPE_STR_DBL_LONG 137
#define SPECASM_LINE_TYPE_STR_HSH_SHORT 138
#define SPECASM_LINE_TYPE_STR_HSH_LONG 139
#define SPECASM_LINE_TYPE_STR_AMP_SHORT 140
#define SPECASM_LINE_TYPE_STR_AMP_LONG 141
#define SPECASM_LINE_TYPE_INC_SHORT 142
#define SPECASM_LINE_TYPE_INC_LONG 143
#define SPECASM_LINE_TYPE_INC_SYS_SHORT 144
#define SPECASM_LINE_TYPE_INC_SYS_LONG 145

#define SPECASM_MAX_MNEMOM 5
#define SPECASM_MAX_LINES 512
#define SPECASM_MAX_ROWS 23
#define SPECASM_LINE_MAX_LEN 32
#define SPECASM_MAX_SCRATCH (SPECASM_LINE_MAX_LEN + 1)
#define SPECASM_NULL 0xff
#define SPECASM_MAX_INDENT 2
#define SPECASM_LINE_MAX_OPCODE                                                \
	(SPECASM_LINE_MAX_LEN - ((SPECASM_MAX_SHORT_LEN + 1)))
#define specasm_short_string_offset(i) (((i) << 3) + ((i) << 2))
#define specasm_short_long_offset(i) ((i) << 5)

#define SPECASM_FLAGS_ADDR_SHORT 0x4
#define SPECASM_FLAGS_ADDR_LONG 0x8
#define SPECASM_FLAGS_ADDR_NUM 0xC
#define SPECASM_FLAGS_NUM_CHAR 0x00
#define SPECASM_FLAGS_NUM_UNSIGNED 0x40
#define SPECASM_FLAGS_NUM_HEX 0x80
#define SPECASM_FLAGS_NUM_SIGNED 0xC0

#define specasm_line_set_size(l, s) ((l)->flags |= s)
#define specasm_line_get_size(l) ((l)->flags & 0x3)
#define specasm_line_get_addr_type(l) (((l)->flags) & 0xC)
#define specasm_line_set_addr_type(l, a) ((l)->flags |= (a))
#define specasm_line_get_format(l) (((l)->flags) & 0xC0)
#define specasm_line_set_format(l, s) ((l)->flags |= (s))
#define specasm_line_get_format2(l) ((((l)->flags) & 0x30) << 2)
#define specasm_line_set_format2(l, s) ((l)->flags |= ((s) >> 2))

/*
 * flags bit field
 * 0-1 size
 * 2-3 address type
 * 4-5 num2 format (only used for ld imm)
 * 6-7 num format
 */

struct specasm_line_t_ {
	uint8_t type;
	uint8_t flags;
	union {
		uint8_t op_code[4];
		uint8_t label;
		uint8_t bad_comment;
	} data;
	uint8_t comment;
};
typedef struct specasm_line_t_ specasm_line_t;

struct specasm_lines_t_ {
	specasm_line_t lines[SPECASM_MAX_LINES];
	uint16_t num_lines;
};

typedef struct specasm_lines_t_ specasm_lines_t;

void specasm_init_dump_table(void);
char *specasm_get_long_imm_e(const char *str, long *val, uint8_t *flags);
uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line);

uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf);

#endif
