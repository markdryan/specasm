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
#include <input.h>
#include <z80.h>

#include "doc.h"
#include "editor.h"
#include "line_common.h"
#include "peer.h"
#include "scratch.h"

#include <string.h>

#define SPECASM_DOC_ENCODING_X 16
#define SPECASM_DOC_M_CYCLES_X 27
#define SPECASM_DOC_T_STATES_X 30

struct specasm_ins_form_t_ {
	const char *form;
	const char *encoding;
	uint8_t m_cycles;
	uint8_t t_states;
};

typedef struct specasm_ins_form_t_ specasm_ins_form_t;

#define SPECASM_DOC_ADC_FORMS 0
#define SPECASM_DOC_ADC_NUM_FORMS 6
#define SPECASM_DOC_ADD1_FORMS \
	(SPECASM_DOC_ADC_FORMS + SPECASM_DOC_ADC_NUM_FORMS)
#define SPECASM_DOC_ADD1_NUM_FORMS 5
#define SPECASM_DOC_ADD2_FORMS \
	(SPECASM_DOC_ADD1_FORMS + SPECASM_DOC_ADD1_NUM_FORMS)
#define SPECASM_DOC_ADD2_NUM_FORMS 1
#define SPECASM_DOC_ADD3_FORMS \
	(SPECASM_DOC_ADD2_FORMS + SPECASM_DOC_ADD2_NUM_FORMS)
#define SPECASM_DOC_ADD3_NUM_FORMS 1
#define SPECASM_DOC_ADD4_FORMS \
	(SPECASM_DOC_ADD3_FORMS + SPECASM_DOC_ADD3_NUM_FORMS)
#define SPECASM_DOC_ADD4_NUM_FORMS 1
#define SPECASM_DOC_ALIGN_FORMS \
	(SPECASM_DOC_ADD4_FORMS + SPECASM_DOC_ADD4_NUM_FORMS)
#define SPECASM_DOC_ALIGN_NUM_FORMS 1
#define SPECASM_DOC_AND_FORMS \
	(SPECASM_DOC_ALIGN_FORMS + SPECASM_DOC_ALIGN_NUM_FORMS)
#define SPECASM_DOC_AND_NUM_FORMS 5
#define SPECASM_DOC_BIT_FORMS \
	(SPECASM_DOC_AND_FORMS + SPECASM_DOC_AND_NUM_FORMS)
#define SPECASM_DOC_BIT_NUM_FORMS 4
#define SPECASM_DOC_CALL_FORMS \
	(SPECASM_DOC_BIT_FORMS + SPECASM_DOC_BIT_NUM_FORMS)
#define SPECASM_DOC_CALL_NUM_FORMS 3
#define SPECASM_DOC_CCF_FORMS \
	(SPECASM_DOC_CALL_FORMS + SPECASM_DOC_CALL_NUM_FORMS)
#define SPECASM_DOC_CCF_NUM_FORMS 1
#define SPECASM_DOC_CP_FORMS \
	(SPECASM_DOC_CCF_FORMS + SPECASM_DOC_CCF_NUM_FORMS)
#define SPECASM_DOC_CP_NUM_FORMS 5
#define SPECASM_DOC_CPD_FORMS \
	(SPECASM_DOC_CP_FORMS + SPECASM_DOC_CP_NUM_FORMS)
#define SPECASM_DOC_CPD_NUM_FORMS 1
#define SPECASM_DOC_CPDR_FORMS \
	(SPECASM_DOC_CPD_FORMS + SPECASM_DOC_CPD_NUM_FORMS)
#define SPECASM_DOC_CPDR_NUM_FORMS 2
#define SPECASM_DOC_CPI_FORMS \
	(SPECASM_DOC_CPDR_FORMS + SPECASM_DOC_CPDR_NUM_FORMS)
#define SPECASM_DOC_CPI_NUM_FORMS 1
#define SPECASM_DOC_CPIR_FORMS \
	(SPECASM_DOC_CPI_FORMS + SPECASM_DOC_CPI_NUM_FORMS)
#define SPECASM_DOC_CPIR_NUM_FORMS 2
#define SPECASM_DOC_CPL_FORMS \
	(SPECASM_DOC_CPIR_FORMS + SPECASM_DOC_CPIR_NUM_FORMS)
#define SPECASM_DOC_CPL_NUM_FORMS 1
#define SPECASM_DOC_DAA_FORMS \
	(SPECASM_DOC_CPL_FORMS + SPECASM_DOC_CPL_NUM_FORMS)
#define SPECASM_DOC_DAA_NUM_FORMS 1
#define SPECASM_DOC_DB_FORMS \
	(SPECASM_DOC_DAA_FORMS + SPECASM_DOC_DAA_NUM_FORMS)
#define SPECASM_DOC_DB_NUM_FORMS 4
#define SPECASM_DOC_DEC1_FORMS \
	(SPECASM_DOC_DB_FORMS + SPECASM_DOC_DB_NUM_FORMS)
#define SPECASM_DOC_DEC1_NUM_FORMS 4
#define SPECASM_DOC_DEC2_FORMS \
	(SPECASM_DOC_DEC1_FORMS + SPECASM_DOC_DEC1_NUM_FORMS)
#define SPECASM_DOC_DEC2_NUM_FORMS 3
#define SPECASM_DOC_DI_FORMS \
	(SPECASM_DOC_DEC2_FORMS + SPECASM_DOC_DEC2_NUM_FORMS)
#define SPECASM_DOC_DI_NUM_FORMS 1
#define SPECASM_DOC_DJNZ_FORMS \
	(SPECASM_DOC_DI_FORMS + SPECASM_DOC_DI_NUM_FORMS)
#define SPECASM_DOC_DJNZ_NUM_FORMS 2
#define SPECASM_DOC_DS_FORMS \
	(SPECASM_DOC_DJNZ_FORMS + SPECASM_DOC_DJNZ_NUM_FORMS)
#define SPECASM_DOC_DS_NUM_FORMS 1
#define SPECASM_DOC_DW_FORMS \
	(SPECASM_DOC_DS_FORMS + SPECASM_DOC_DS_NUM_FORMS)
#define SPECASM_DOC_DW_NUM_FORMS 2
#define SPECASM_DOC_EI_FORMS \
	(SPECASM_DOC_DW_FORMS + SPECASM_DOC_DW_NUM_FORMS)
#define SPECASM_DOC_EI_NUM_FORMS 1
#define SPECASM_DOC_EX_FORMS \
	(SPECASM_DOC_EI_FORMS + SPECASM_DOC_EI_NUM_FORMS)
#define SPECASM_DOC_EX_NUM_FORMS 5
#define SPECASM_DOC_EXX_FORMS \
	(SPECASM_DOC_EX_FORMS + SPECASM_DOC_EX_NUM_FORMS)
#define SPECASM_DOC_EXX_NUM_FORMS 1
#define SPECASM_DOC_HALT_FORMS \
	(SPECASM_DOC_EXX_FORMS + SPECASM_DOC_EXX_NUM_FORMS)
#define SPECASM_DOC_HALT_NUM_FORMS 1
#define SPECASM_DOC_IM_FORMS \
	(SPECASM_DOC_HALT_FORMS + SPECASM_DOC_HALT_NUM_FORMS)
#define SPECASM_DOC_IM_NUM_FORMS 3
#define SPECASM_DOC_IN1_FORMS \
	(SPECASM_DOC_IM_FORMS + SPECASM_DOC_IM_NUM_FORMS)
#define SPECASM_DOC_IN1_NUM_FORMS 1
#define SPECASM_DOC_IN2_FORMS \
	(SPECASM_DOC_IN1_FORMS + SPECASM_DOC_IN1_NUM_FORMS)
#define SPECASM_DOC_IN2_NUM_FORMS 1
#define SPECASM_DOC_INC1_FORMS \
	(SPECASM_DOC_IN2_FORMS + SPECASM_DOC_IN2_NUM_FORMS)
#define SPECASM_DOC_INC1_NUM_FORMS 4
#define SPECASM_DOC_INC2_FORMS \
	(SPECASM_DOC_INC1_FORMS + SPECASM_DOC_INC1_NUM_FORMS)
#define SPECASM_DOC_INC2_NUM_FORMS 3
#define SPECASM_DOC_IND_FORMS \
	(SPECASM_DOC_INC2_FORMS + SPECASM_DOC_INC2_NUM_FORMS)
#define SPECASM_DOC_IND_NUM_FORMS 1
#define SPECASM_DOC_INDR_FORMS \
	(SPECASM_DOC_IND_FORMS + SPECASM_DOC_IND_NUM_FORMS)
#define SPECASM_DOC_INDR_NUM_FORMS 2
#define SPECASM_DOC_INI_FORMS \
	(SPECASM_DOC_INDR_FORMS + SPECASM_DOC_INDR_NUM_FORMS)
#define SPECASM_DOC_INI_NUM_FORMS 1
#define SPECASM_DOC_INIR_FORMS \
	(SPECASM_DOC_INI_FORMS + SPECASM_DOC_INI_NUM_FORMS)
#define SPECASM_DOC_INIR_NUM_FORMS 2
#define SPECASM_DOC_JP_FORMS \
	(SPECASM_DOC_INIR_FORMS + SPECASM_DOC_INIR_NUM_FORMS)
#define SPECASM_DOC_JP_NUM_FORMS 6
#define SPECASM_DOC_LDD_FORMS \
	(SPECASM_DOC_JP_FORMS + SPECASM_DOC_JP_NUM_FORMS)
#define SPECASM_DOC_LDD_NUM_FORMS 1
#define SPECASM_DOC_LDDR_FORMS \
	(SPECASM_DOC_LDD_FORMS + SPECASM_DOC_LDD_NUM_FORMS)
#define SPECASM_DOC_LDDR_NUM_FORMS 2
#define SPECASM_DOC_LDI_FORMS \
	(SPECASM_DOC_LDDR_FORMS + SPECASM_DOC_LDDR_NUM_FORMS)
#define SPECASM_DOC_LDI_NUM_FORMS 1
#define SPECASM_DOC_LDIR_FORMS \
	(SPECASM_DOC_LDI_FORMS + SPECASM_DOC_LDI_NUM_FORMS)
#define SPECASM_DOC_LDIR_NUM_FORMS 2
#define SPECASM_DOC_MAP_FORMS \
	(SPECASM_DOC_LDIR_FORMS + SPECASM_DOC_LDIR_NUM_FORMS)
#define SPECASM_DOC_MAP_NUM_FORMS 1
#define SPECASM_DOC_NEG_FORMS \
	(SPECASM_DOC_MAP_FORMS + SPECASM_DOC_MAP_NUM_FORMS)
#define SPECASM_DOC_NEG_NUM_FORMS 1
#define SPECASM_DOC_NOP_FORMS \
	(SPECASM_DOC_NEG_FORMS + SPECASM_DOC_NEG_NUM_FORMS)
#define SPECASM_DOC_NOP_NUM_FORMS 1
#define SPECASM_DOC_OR_FORMS \
	(SPECASM_DOC_NOP_FORMS + SPECASM_DOC_NOP_NUM_FORMS)
#define SPECASM_DOC_OR_NUM_FORMS 5
#define SPECASM_DOC_ORG_FORMS \
	(SPECASM_DOC_OR_FORMS + SPECASM_DOC_OR_NUM_FORMS)
#define SPECASM_DOC_ORG_NUM_FORMS 1
#define SPECASM_DOC_OUT1_FORMS \
	(SPECASM_DOC_ORG_FORMS + SPECASM_DOC_ORG_NUM_FORMS)
#define SPECASM_DOC_OUT1_NUM_FORMS 1
#define SPECASM_DOC_OUT2_FORMS \
	(SPECASM_DOC_OUT1_FORMS + SPECASM_DOC_OUT1_NUM_FORMS)
#define SPECASM_DOC_OUT2_NUM_FORMS 1
#define SPECASM_DOC_OUTD_FORMS \
	(SPECASM_DOC_OUT2_FORMS + SPECASM_DOC_OUT2_NUM_FORMS)
#define SPECASM_DOC_OUTD_NUM_FORMS 1
#define SPECASM_DOC_OUTDR_FORMS \
	(SPECASM_DOC_OUTD_FORMS + SPECASM_DOC_OUTD_NUM_FORMS)
#define SPECASM_DOC_OUTDR_NUM_FORMS 2
#define SPECASM_DOC_OUTI_FORMS \
	(SPECASM_DOC_OUTDR_FORMS + SPECASM_DOC_OUTDR_NUM_FORMS)
#define SPECASM_DOC_OUTI_NUM_FORMS 1
#define SPECASM_DOC_OUTIR_FORMS \
	(SPECASM_DOC_OUTI_FORMS + SPECASM_DOC_OUTI_NUM_FORMS)
#define SPECASM_DOC_OUTIR_NUM_FORMS 2
#define SPECASM_DOC_POP_FORMS \
	(SPECASM_DOC_OUTIR_FORMS + SPECASM_DOC_OUTIR_NUM_FORMS)
#define SPECASM_DOC_POP_NUM_FORMS 3
#define SPECASM_DOC_PUSH_FORMS \
	(SPECASM_DOC_POP_FORMS + SPECASM_DOC_POP_NUM_FORMS)
#define SPECASM_DOC_PUSH_NUM_FORMS 3
#define SPECASM_DOC_RES_FORMS \
	(SPECASM_DOC_PUSH_FORMS + SPECASM_DOC_PUSH_NUM_FORMS)
#define SPECASM_DOC_RES_NUM_FORMS 4
#define SPECASM_DOC_RET_FORMS \
	(SPECASM_DOC_RES_FORMS + SPECASM_DOC_RES_NUM_FORMS)
#define SPECASM_DOC_RET_NUM_FORMS 3
#define SPECASM_DOC_RETI_FORMS \
	(SPECASM_DOC_RET_FORMS + SPECASM_DOC_RET_NUM_FORMS)
#define SPECASM_DOC_RETI_NUM_FORMS 1
#define SPECASM_DOC_RETN_FORMS \
	(SPECASM_DOC_RETI_FORMS + SPECASM_DOC_RETI_NUM_FORMS)
#define SPECASM_DOC_RETN_NUM_FORMS 1
#define SPECASM_DOC_RL_FORMS \
	(SPECASM_DOC_RETN_FORMS + SPECASM_DOC_RETN_NUM_FORMS)
#define SPECASM_DOC_RL_NUM_FORMS 4
#define SPECASM_DOC_RLA_FORMS \
	(SPECASM_DOC_RL_FORMS + SPECASM_DOC_RL_NUM_FORMS)
#define SPECASM_DOC_RLA_NUM_FORMS 1
#define SPECASM_DOC_RLC_FORMS \
	(SPECASM_DOC_RLA_FORMS + SPECASM_DOC_RLA_NUM_FORMS)
#define SPECASM_DOC_RLC_NUM_FORMS 4
#define SPECASM_DOC_RLCA_FORMS \
	(SPECASM_DOC_RLC_FORMS + SPECASM_DOC_RLC_NUM_FORMS)
#define SPECASM_DOC_RLCA_NUM_FORMS 1
#define SPECASM_DOC_RLD_FORMS \
	(SPECASM_DOC_RLCA_FORMS + SPECASM_DOC_RLCA_NUM_FORMS)
#define SPECASM_DOC_RLD_NUM_FORMS 1
#define SPECASM_DOC_RR_FORMS \
	(SPECASM_DOC_RLD_FORMS + SPECASM_DOC_RLD_NUM_FORMS)
#define SPECASM_DOC_RR_NUM_FORMS 4
#define SPECASM_DOC_RRA_FORMS \
	(SPECASM_DOC_RR_FORMS + SPECASM_DOC_RR_NUM_FORMS)
#define SPECASM_DOC_RRA_NUM_FORMS 1
#define SPECASM_DOC_RRC_FORMS \
	(SPECASM_DOC_RRA_FORMS + SPECASM_DOC_RRA_NUM_FORMS)
#define SPECASM_DOC_RRC_NUM_FORMS 4
#define SPECASM_DOC_RRCA_FORMS \
	(SPECASM_DOC_RRC_FORMS + SPECASM_DOC_RRC_NUM_FORMS)
#define SPECASM_DOC_RRCA_NUM_FORMS 1
#define SPECASM_DOC_RRD_FORMS \
	(SPECASM_DOC_RRCA_FORMS + SPECASM_DOC_RRCA_NUM_FORMS)
#define SPECASM_DOC_RRD_NUM_FORMS 1
#define SPECASM_DOC_RST_FORMS \
	(SPECASM_DOC_RRD_FORMS + SPECASM_DOC_RRD_NUM_FORMS)
#define SPECASM_DOC_RST_NUM_FORMS 1
#define SPECASM_DOC_SBC_FORMS \
	(SPECASM_DOC_RST_FORMS + SPECASM_DOC_RST_NUM_FORMS)
#define SPECASM_DOC_SBC_NUM_FORMS 6
#define SPECASM_DOC_SCF_FORMS \
	(SPECASM_DOC_SBC_FORMS + SPECASM_DOC_SBC_NUM_FORMS)
#define SPECASM_DOC_SCF_NUM_FORMS 1
#define SPECASM_DOC_SET_FORMS \
	(SPECASM_DOC_SCF_FORMS + SPECASM_DOC_SCF_NUM_FORMS)
#define SPECASM_DOC_SET_NUM_FORMS 4
#define SPECASM_DOC_SLA_FORMS \
	(SPECASM_DOC_SET_FORMS + SPECASM_DOC_SET_NUM_FORMS)
#define SPECASM_DOC_SLA_NUM_FORMS 4
#define SPECASM_DOC_SRA_FORMS \
	(SPECASM_DOC_SLA_FORMS + SPECASM_DOC_SLA_NUM_FORMS)
#define SPECASM_DOC_SRA_NUM_FORMS 4
#define SPECASM_DOC_SRL_FORMS \
	(SPECASM_DOC_SRA_FORMS + SPECASM_DOC_SRA_NUM_FORMS)
#define SPECASM_DOC_SRL_NUM_FORMS 4
#define SPECASM_DOC_SUB_FORMS \
	(SPECASM_DOC_SRL_FORMS + SPECASM_DOC_SRL_NUM_FORMS)
#define SPECASM_DOC_SUB_NUM_FORMS 5
#define SPECASM_DOC_XOR_FORMS \
	(SPECASM_DOC_SUB_FORMS + SPECASM_DOC_SUB_NUM_FORMS)
#define SPECASM_DOC_XOR_NUM_FORMS 5


static const specasm_ins_form_t specasm_forms[] = {
	/* SPECASM_DOC_ADC_FORMS */
	{"a,r", "88+r", 1, 4},
	{"a,n", "CE n",  2, 7},
	{"a,(hl)", "8E", 2, 7},
	{"a,(ix+d)", "DD 8E d", 5, 19 },
	{"a,(iy+d)", "FD 8E d", 5, 19 },
	{"hl,rr", "ED 4A+rr", 4, 15 },

	/* SPECASM_DOC_ADD1_FORMS */
	{"a,r", "80+r", 1, 4},
	{"a,n", "C6 n",  2, 7},
	{"a,(hl)", "86", 2, 7},
	{"a,(ix+d)", "DD 86 d", 5, 19 },
	{"a,(iy+d)", "FD 86 d", 5, 19 },

	/* SPECASM_DOC_ADD2_FORMS */
	{"hl,rr", "9+rr", 3, 11 },

	/* SPECASM_DOC_ADD3_FORMS */
	{"ix,rr", "DD 9+rr", 4, 15 },

	/* SPECASM_DOC_ADD4_FORMS */
	{"iy,rr", "FD 9+rr", 4, 15 },

	/* SPECASM_DOC_ALIGN_FORMS */
	{"n", "0", 1, 4 },

	/* SPECASM_DOC_AND_FORMS */
	{"r", "A0+r", 1, 4},
	{"n", "E6 n",  2, 7},
	{"(hl)", "A6", 2, 7},
	{"(ix+d)", "DD A6 d", 5, 19 },
	{"(iy+d)", "FD A6 d", 5, 19 },

	/* SPECASM_DOC_BIT_FORMS */
	{"n,r", "CB 40+b+r", 2, 8},
	{"n,(hl)", "CB 46+b", 3, 12},
	{"n,(ix+d)", "DDCBd46+b", 5, 20},
	{"n,(iy+d)", "FDCBd46+b", 5, 20},

	/* SPECASM_DOC_CALL_FORMS */
	{"nn", "CD n n", 5, 17},
	{"cc,nn", "C4+cc n n", 5, 17},
	{"cc,nn", "C4+cc n n", 3, 10},

	/* SPECASM_DOC_CCF_FORMS */
	{"", "3F", 1, 4},

	/* SPECASM_DOC_CP_FORMS */
	{"r", "B8+r", 1, 4},
	{"n", "FE n",  2, 7},
	{"(hl)", "BE", 2, 7},
	{"(ix+d)", "DD BE d", 5, 19 },
	{"(iy+d)", "FD BE d", 5, 19 },

	/* SPECASM_DOC_CPD_FORMS */
	{"", "ED A9", 4, 16},

	/* SPECASM_DOC_CPDR_FORMS */
	{"", "ED B9", 4, 16},
	{"", "ED B9", 5, 21},

	/* SPECASM_DOC_CPI_FORMS */
	{"", "ED A1", 4, 16},

	/* SPECASM_DOC_CPIR_FORMS */
	{"", "ED B1", 4, 16},
	{"", "ED B1", 5, 21},

	/* SPECASM_DOC_CPL_FORMS */
	{"", "2F", 1, 4},

	/* SPECASM_DOC_DAA_FORMS */
	{"", "27", 1, 4},

	/* SPECASM_DOC_DB_FORMS */
	{"n", "n", 0, 0},
	{"n,n", "n n", 0, 0},
	{"n,n,n", "n n n", 0, 0},
	{"n,n,n,n", "n n n n", 0, 0},

	/* SPECASM_DOC_DEC1_FORMS */
	{"r", "5+r", 1, 4},
	{"(hl)", "35", 3, 11},
	{"(ix+d)", "DD 35 d", 6, 23 },
	{"(iy+d)", "FD 35 d", 6, 23 },

	/* SPECASM_DOC_DEC2_FORMS */
	{"rr", "B+rr", 1, 6},
	{"ix", "DD 2B", 2, 10 },
	{"iy", "FD 2B", 2, 10 },

	/* SPECASM_DOC_DI_FORMS */
	{"", "F3", 1, 4},

	/* SPECASM_DOC_DJNZ_FORMS */
	{"n", "10 n", 2, 8},
	{"n", "10 n", 3, 13},

	/* SPECASM_DOC_DS_FORMS */
	{"c n", "n c times", 0, 0},

	/* SPECASM_DOC_DW_FORMS */
	{"nn", "nn", 0, 0},
	{"nn,nn", "nn nn", 0, 0},

	/* SPECASM_DOC_EI_FORMS */
	{"", "FB", 1, 4},

	/* SPECASM_DOC_EX_FORMS */
	{"af, af'", "08", 1, 4},
	{"de, hl", "EB", 1, 4},
	{"sp, (hl)", "E3", 5, 19},
	{"(sp), ix", "DD E3", 6, 23},
	{"(sp), iy", "FD E3", 6, 23},

	/* SPECASM_DOC_EXX_FORMS */
	{"", "D9", 1, 4},

	/* SPECASM_DOC_HALT_FORMS */
	{"", "76", 1, 4},

	/* SPECASM_DOC_IM_FORMS */
	{"0", "ED 46", 2, 8},
	{"1", "ED 56", 2, 8},
	{"2", "ED 5E", 2, 8},

	/* SPECASM_DOC_IN1_FORMS */
	{"r, (c)", "ED 40+r", 3, 12},

	/* SPECASM_DOC_IN2_FORMS */
	{"a, (n)", "DB n", 3, 11},

	/* SPECASM_DOC_INC1_FORMS */
	{"r", "4+r", 1, 4},
	{"(hl)", "34", 3, 11},
	{"(ix+d)", "DD 34 d", 6, 23 },
	{"(iy+d)", "FD 34 d", 6, 23 },

	/* SPECASM_DOC_INC2_FORMS */
	{"rr", "3+rr", 1, 6},
	{"ix", "DD 23", 2, 10 },
	{"iy", "FD 23", 2, 10 },

	/* SPECASM_DOC_IND_FORMS */
	{"", "ED AA", 4, 16},

	/* SPECASM_DOC_INDR_FORMS */
	{"", "ED BA", 4, 16},
	{"", "ED BA", 5, 21},

	/* SPECASM_DOC_INI_FORMS */
	{"", "ED A2", 4, 16},

	/* SPECASM_DOC_INIR_FORMS */
	{"", "ED B2", 4, 16},
	{"", "ED B2", 5, 21},

	/* SPECASM_DOC_JP_FORMS */
	{"nn", "C3 n n", 3, 10},
	{"(hl)", "E9", 1, 4},
	{"(ix)", "DD E9", 2, 8},
	{"(iy)", "FD E9", 2, 8},
	{"cc,nn", "C2+cc n n", 3, 12},
	{"cc,nn", "C2+cc n n", 2, 7},

	/* SPECASM_DOC_LDD_FORMS */
	{"", "ED A8", 4, 16},

	/* SPECASM_DOC_LDDR_FORMS */
	{"", "ED B8", 4, 16},
	{"", "ED B8", 5, 21},

	/* SPECASM_DOC_LDI_FORMS */
	{"", "ED A0", 4, 16},

	/* SPECASM_DOC_LDIR_FORMS */
	{"", "ED B0", 4, 16},
	{"", "ED B0", 5, 21},

	/* SPECASM_DOC_MAP_FORMS */
	{"", "", 0, 0},

	/* SPECASM_DOC_NEG_FORMS */
	{"", "ED 44", 2, 8},

	/* SPECASM_DOC_NOP_FORMS */
	{"", "0", 1, 4},

	/* SPECASM_DOC_OR_FORMS */
	{"r", "B8+r", 1, 4},
	{"n", "F6 n",  2, 7},
	{"(hl)", "B6", 2, 7},
	{"(ix+d)", "DD B6 d", 5, 19 },
	{"(iy+d)", "FD B6 d", 5, 19 },

	/* SPECASM_DOC_ORG_FORMS */
	{"nn","", 0, 0},

	/* SPECASM_DOC_OUT1_FORMS */
	{"(c), r", "ED 41+r", 3, 12},

	/* SPECASM_DOC_OUT2_FORMS */
	{"(n), a", "D3 n", 3, 11},

	/* SPECASM_DOC_OUTD_FORMS */
	{"","ED AB", 4, 16},

	/* SPECASM_DOC_OUTDR_FORMS */
	{"","ED BB", 4, 16},
	{"","ED BB", 5, 21},

	/* SPECASM_DOC_OUTI_FORMS */
	{"","ED A3", 4, 16},

	/* SPECASM_DOC_OUTIR_FORMS */
	{"","ED B3", 4, 16},
	{"","ED B3", 5, 21},

	/* SPECASM_DOC_POP_FORMS */
	{"rr", "C1+rr", 3, 10},
	{"ix", "DD E1", 4, 14},
	{"iy", "FD E1", 4, 14},

	/* SPECASM_DOC_PUSH_FORMS */
	{"rr", "C5+rr", 3, 11},
	{"ix", "DD E5", 4, 15},
	{"iy", "FD E5", 4, 15},

	/* SPECASM_DOC_RES_FORMS */
	{"n,r", "CB 80+b+r", 2, 8},
	{"n,(hl)", "CB 86+b", 4, 15},
	{"n,(ix+d)", "DDCBd86+b", 6, 23},
	{"n,(iy+d)", "FDCBd86+b", 6, 23},

	/* SPECASM_DOC_RET_FORMS */
	{"", "C9", 3, 10},
	{"cc", "C0+cc", 1, 5},
	{"cc", "C0+cc", 3, 10},

	/* SPECASM_DOC_RETI_FORMS */
	{"", "ED 4D", 4, 14},

	/* SPECASM_DOC_RETN_FORMS */
	{"", "ED 45", 4, 14},

	/* SPECASM_DOC_RL_FORMS */
	{"r", "CB 10+r", 2, 8},
	{"(hl)", "CB 16", 4, 15},
	{"(ix+d)", "DD CB d 16", 6, 23 },
	{"(iy+d)", "FD CB d 16", 6, 23 },

	/* SPECASM_DOC_RLA_FORMS */
	{"", "17", 1, 4},

	/* SPECASM_DOC_RLC_FORMS */
	{"r", "CB r", 2, 8},
	{"(hl)", "CB 06", 4, 15},
	{"(ix+d)", "DD CB d 06", 6, 23 },
	{"(iy+d)", "FD CB d 06", 6, 23 },

	/* SPECASM_DOC_RLCA_FORMS */
	{"", "07", 1, 4},

	/* SPECASM_DOC_RLD_FORMS */
	{"", "ED 6F", 5, 18},

	/* SPECASM_DOC_RR_FORMS */
	{"r", "CB 18+r", 2, 8},
	{"(hl)", "CB 1E", 4, 15},
	{"(ix+d)", "DD CB d 1E", 6, 23 },
	{"(iy+d)", "FD CB d 1E", 6, 23 },

	/* SPECASM_DOC_RRA_FORMS */
	{"", "1F", 1, 4},

	/* SPECASM_DOC_RRC_FORMS */
	{"r", "CB 8+r", 2, 8},
	{"(hl)", "CB 0E", 4, 15},
	{"(ix+d)", "DD CB d 0E", 6, 23 },
	{"(iy+d)", "FD CB d 0E", 6, 23 },

	/* SPECASM_DOC_RRCA_FORMS */
	{"", "0F", 1, 4},

	/* SPECASM_DOC_RRD_FORMS */
	{"", "ED 67", 5, 18},

	/* SPECASM_DOC_RST_FORMS */
	{"n", "C3+n", 3, 11},

	/* SPECASM_DOC_SBC_FORMS */
	{"a,r", "98+r", 1, 4},
	{"a,n", "DE n",  2, 7},
	{"a,(hl)", "9E", 2, 7},
	{"a,(ix+d)", "DD 9E d", 5, 19 },
	{"a,(iy+d)", "FD 9E d", 5, 19 },
	{"hl,rr", "ED 42+rr", 4, 15 },

	/* SPECASM_DOC_SCF_FORMS */
	{"", "37", 1, 4},

	/* SPECASM_DOC_SET_FORMS */
	{"n,r", "CB C0+b+r", 2, 8},
	{"n,(hl)", "CB C6+b", 4, 15},
	{"n,(ix+d)", "DDCBdC6+b", 6, 23},
	{"n,(iy+d)", "FDCBdC6+b", 6, 23},

	/* SPECASM_DOC_SLA_FORMS */
	{"r", "CB 20+r", 2, 8},
	{"(hl)", "CB 26", 4, 15},
	{"(ix+d)", "DD CB d 26", 6, 23 },
	{"(iy+d)", "FD CB d 26", 6, 23 },

	/* SPECASM_DOC_SRA_FORMS */
	{"r", "CB 28+r", 2, 8},
	{"(hl)", "CB 2E", 4, 15},
	{"(ix+d)", "DD CB d 2E", 6, 23 },
	{"(iy+d)", "FD CB d 2E", 6, 23 },

	/* SPECASM_DOC_SRL_FORMS */
	{"r", "CB 38+r", 2, 8},
	{"(hl)", "CB 3E", 4, 15},
	{"(ix+d)", "DD CB d 3E", 6, 23 },
	{"(iy+d)", "FD CB d 3E", 6, 23 },

	/* SPECASM_DOC_SUB_FORMS */
	{"r", "A0+r", 1, 4},
	{"n", "D6 n",  2, 7},
	{"(hl)", "96", 2, 7},
	{"(ix+d)", "DD 96 d", 5, 19 },
	{"(iy+d)", "FD 96 d", 5, 19 },

	/* SPECASM_DOC_XOR_FORMS */
	{"r", "A8+r", 1, 4},
	{"n", "EE n",  2, 7},
	{"(hl)", "AE", 2, 7},
	{"(ix+d)", "DD AE d", 5, 19 },
	{"(iy+d)", "FD AE d", 5, 19 },
};

#define SPECASM_DOC_MAX_REG_ENCODING (SPECASM_BYTE_REG_IY+1)

struct specasm_ins_doc_t_ {
	char name[8];
	uint8_t forms;
	uint8_t num_forms;
	uint8_t reg_encoding;
	uint8_t bits;
	uint8_t all_cc;
	const char *description;
	const char *flags;
};

typedef struct specasm_ins_doc_t_ specasm_ins_doc_t;

static const char* const reg_names[] = {
	"a",
	"b",
	"c",
	"d",
	"e",
	"h",
	"l",
	"bc",
	"de",
	"hl",
	"af",
	"sp",
	"ix",
	"iy"
};

const uint8_t reg_encodings[][SPECASM_DOC_MAX_REG_ENCODING] = {
	{ 8, 1, 2, 3, 4, 5, 6, 1, 17, 33, 0, 49 },
	{ 8, 1, 2, 3, 4, 5, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 33, 0, 49 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 0, 0, 49, 33 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 0, 0, 49, 0, 33},
	{ 57, 1, 9, 17, 25, 33, 49},
	{ 0, 0, 0, 0, 0, 0, 0, 1, 17, 33, 49 },
};

/*
 * Must be in the same order as the opcode_table in line_parse.c.
 */

const static char specasm_doc_add_16[] =
	"The second argument is added to the contents of the "
	"destination register. The carry flag is set according "
	"to the result of the addition. h represents carry from "
	"bit 11.";

const static char specasm_doc_inc[] =
	"The specified operand is incremented by 1.";

const static specasm_ins_doc_t docs[] = {
	{
		"adc",
		SPECASM_DOC_ADC_FORMS,
		SPECASM_DOC_ADC_NUM_FORMS,
		1,
		0,
		0,
		"The second argument and the carry flag are added to the "
		"contents of the destination register.",
		"XX X X0X",
	},
	{
		"add",
		SPECASM_DOC_ADD1_FORMS,
		SPECASM_DOC_ADD1_NUM_FORMS,
		2,
		0,
		0,
		"The second argument is added to the contents of the "
		"destination register.",
		"XX X X0X",
	},
	{
		"add",
		SPECASM_DOC_ADD2_FORMS,
		SPECASM_DOC_ADD2_NUM_FORMS,
		3,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"add",
		SPECASM_DOC_ADD3_FORMS,
		SPECASM_DOC_ADD3_NUM_FORMS,
		4,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"add",
		SPECASM_DOC_ADD4_FORMS,
		SPECASM_DOC_ADD4_NUM_FORMS,
		5,
		0,
		0,
		specasm_doc_add_16,
		"   X  0X",
	},
	{
		"align",
		SPECASM_DOC_ALIGN_FORMS,
		SPECASM_DOC_ALIGN_NUM_FORMS,
		0,
		0,
		0,
		"The align directive takes one immediate argument that must be "
		"a power of 2, >= 2 and <= 256.  It inserts null bytes into "
		"the binary until the requested alignment is achieved. The "
		"number of t-states consumed by an align directive is the "
		"number of bytes inserted * 4.",
		NULL,
	},
	{
		"and",
		SPECASM_DOC_AND_FORMS,
		SPECASM_DOC_AND_NUM_FORMS,
		2,
		0,
		0,
		"The result of a bitwise AND of a and the "
		"argument is stored in a.",
		"XX 1 X00",
	},
	{
		"bit",
		SPECASM_DOC_BIT_FORMS,
		SPECASM_DOC_BIT_NUM_FORMS,
		2,
		8,
		0,
		"Sets the zero flag to 1 if bit n of the 2nd operand "
		"is 0, or to 0 if bit N is 1.",
		"?X 1 ?0 ",
	},
	{
		"call",
		SPECASM_DOC_CALL_FORMS,
		SPECASM_DOC_CALL_NUM_FORMS,
		0,
		0,
		8,
		"Pushes the PC on the stack and jumps to nn. The conditional "
		"version of the instruction takes fewer t-states to execute "
		"if the call is not taken."
		,
		NULL,
	},
	{
		"ccf",
		SPECASM_DOC_CCF_FORMS,
		SPECASM_DOC_CCF_NUM_FORMS,
		0,
		0,
		0,
		"Inverts the carry flag.",
		"   ?  0X"
	},
	{
		"cp",
		SPECASM_DOC_CP_FORMS,
		SPECASM_DOC_CP_NUM_FORMS,
		2,
		0,
		0,
		"The operand is subtracted from a setting the "
		"flags accordingly. The result of the subtraction is "
		"discarded.",
		"XX X X1X",
	},
	{
		"cpd",
		SPECASM_DOC_CPD_FORMS,
		SPECASM_DOC_CPD_NUM_FORMS,
		0,
		0,
		0,
		"(hl) is subtracted from a and the flags are set "
		"accordingly. The result is discarded. bc and hl are "
		"decremented. The p flag is set if bc != 0 after the "
		"instruction has finished and is otherwise reset.",
		"XX X X1 ",
	},
	{
		"cpdr",
		SPECASM_DOC_CPDR_FORMS,
		SPECASM_DOC_CPDR_NUM_FORMS,
		0,
		0,
		0,
		"(hl) is subtracted from a and the flags are set accordingly. "
		"The result is discarded. bc and hl are decremented. If bc>0 "
		"and the result of the subtraction is != 0 cpdr "
		"repeats. The p flag is set if bc!=0 after cpdr has "
		"finished and is otherwise reset. The slower timings apply "
		"when cpdr repeats.",
		"XX X X1 ",
	},
	{
		"cpi",
		SPECASM_DOC_CPI_FORMS,
		SPECASM_DOC_CPI_NUM_FORMS,
		0,
		0,
		0,
		"(hl) is subtracted from a and the flags are set accordingly. "
		"The result is discarded. bc is decremented while hl is "
		"incremented. The p flag is set if bc != 0 after the cpi has "
		"finished and is otherwise reset.",
		"XX X X1 ",
	},
	{
		"cpir",
		SPECASM_DOC_CPIR_FORMS,
		SPECASM_DOC_CPIR_NUM_FORMS,
		0,
		0,
		0,
		"(hl) is subtracted from a and the flags are set accordingly. "
		"The result is discarded. bc is decremented while hl is "
		"incremented. If bc>0 and the result of the subtraction is != 0"
		" cpir repeats. The p flag is set if bc!=0 after cpir "
		"has finished and is otherwise reset. The slower "
		"timings apply when the cpir repeats.",
		"XX X X1 ",
	},
	{
		"cpl",
		SPECASM_DOC_CPL_FORMS,
		SPECASM_DOC_CPL_NUM_FORMS,
		0,
		0,
		0,
		"Invert a.",
		"   1  1 ",
	},
	{
		"daa",
		SPECASM_DOC_DAA_FORMS,
		SPECASM_DOC_DAA_NUM_FORMS,
		0,
		0,
		0,
		"Conditionally adjusts a for BCD addition and subtraction.",
		"XX X X X",
	},
	{
		"db",
		SPECASM_DOC_DB_FORMS,
		SPECASM_DOC_DB_NUM_FORMS,
		0,
		0,
		0,
		"Stores up to 4 bytes in the program binary.  All ns must be "
		"formatted in the same way. Only one byte can be specified if "
		"an expression is used.",
		NULL,
	},
	{
		"dec",
		SPECASM_DOC_DEC1_FORMS,
		SPECASM_DOC_DEC1_NUM_FORMS,
		6,
		0,
		0,
		"The specified operand is decremented by 1.",
		"XX X X1 ",
	},
	{
		"dec",
		SPECASM_DOC_DEC2_FORMS,
		SPECASM_DOC_DEC2_NUM_FORMS,
		3,
		0,
		0,
		"The specified operand is decremented by 1.",
		NULL,
	},
	{
		"di",
		SPECASM_DOC_DI_FORMS,
		SPECASM_DOC_DI_NUM_FORMS,
		0,
		0,
		0,
		"Disables maskable interrupts.",
		NULL,
	},
	{
		"djnz",
		SPECASM_DOC_DJNZ_FORMS,
		SPECASM_DOC_DJNZ_NUM_FORMS,
		0,
		0,
		0,
		"b is decremented by 1. If the result is>0 the cpu jumps to "
		"PC+2+n, where n is a signed 8 byte. djnz executes "
		"more quickly when the jump it not taken.",
		NULL,
	},
	{
		"ds",
		SPECASM_DOC_DS_FORMS,
		SPECASM_DOC_DS_NUM_FORMS,
		0,
		0,
		0,
		"Stores c copies of the byte n in the binary.",
		NULL,
	},
	{
		"dw",
		SPECASM_DOC_DW_FORMS,
		SPECASM_DOC_DW_NUM_FORMS,
		0,
		0,
		0,
		"Stores up to 2 words in the program binary.  All nns must be "
		"formatted in the same way. Only one word can be specified if "
		"an expression is used.",
		NULL,
	},
	{
		"ei",
		SPECASM_DOC_EI_FORMS,
		SPECASM_DOC_EI_NUM_FORMS,
		0,
		0,
		0,
		"Enables maskable interrupts.",
		NULL,
	},
	{
		"ex",
		SPECASM_DOC_EX_FORMS,
		SPECASM_DOC_EX_NUM_FORMS,
		0,
		0,
		0,
		"The contents of the two operands are exchanged. "
		"ex af, af' affects all the flags while the other forms of the "
		"instruction have no effect on the flags.",
		"XXXXXXXX",
	},
	{
		"exx",
		SPECASM_DOC_EXX_FORMS,
		SPECASM_DOC_EXX_NUM_FORMS,
		0,
		0,
		0,
		"Exchange bc, de and hl with bc', de', hl'.",
		NULL,
	},
	{
		"halt",
		SPECASM_DOC_HALT_FORMS,
		SPECASM_DOC_HALT_NUM_FORMS,
		0,
		0,
		0,
		"CPU execution is suspended until the next interrupt or reset.",
		NULL,
	},
	{
		"im",
		SPECASM_DOC_IM_FORMS,
		SPECASM_DOC_IM_NUM_FORMS,
		0,
		0,
		0,
		"Sets the interrupt mode.  With im 2 the MSB of the vector "
		"address is taken from the i register.",
		NULL,
	},
	{
		"in",
		SPECASM_DOC_IN1_FORMS,
		SPECASM_DOC_IN1_NUM_FORMS,
		6,
		0,
		0,
		"Reads a byte from the device identified by bc and stores it "
		"in r.",
		"XX X X0 ",
	},
	{
		"in",
		SPECASM_DOC_IN2_FORMS,
		SPECASM_DOC_IN2_NUM_FORMS,
		0,
		0,
		0,
		"Reads a byte from the device adddress whose MSB is "
		"taken from a and whose LSB is n. The byte is "
		"stored in a.",
		NULL,
	},
	{
		"inc",
		SPECASM_DOC_INC1_FORMS,
		SPECASM_DOC_INC1_NUM_FORMS,
		6,
		0,
		0,
		specasm_doc_inc,
		"XX X X0 ",
	},
	{
		"inc",
		SPECASM_DOC_INC2_FORMS,
		SPECASM_DOC_INC2_NUM_FORMS,
		3,
		0,
		0,
		specasm_doc_inc,
		NULL,
	},
	{
		"ind",
		SPECASM_DOC_IND_FORMS,
		SPECASM_DOC_IND_NUM_FORMS,
		0,
		0,
		0,
		"A byte is read from the device identified by bc and stored "
		"in (hl). b and hl are decremented.",
		"?X ? ?1 ",
	},
	{
		"indr",
		SPECASM_DOC_INDR_FORMS,
		SPECASM_DOC_INDR_NUM_FORMS,
		0,
		0,
		0,
		"A byte is read from the device identified by bc and stored "
		"(hl). b and hl are decremented. If bc!=0 indr "
		"repeats. indr consumes more t-states when it "
		"repeats.",
		"?1 ? ?1 "
	},
	{
		"ini",
		SPECASM_DOC_INI_FORMS,
		SPECASM_DOC_INI_NUM_FORMS,
		0,
		0,
		0,
		"A byte is read from the device identified by bc "
		"and stored in (hl). b is decremented and h is incremented.",
		"?X ? ?1 ",
	},
	{
		"inir",
		SPECASM_DOC_INIR_FORMS,
		SPECASM_DOC_INIR_NUM_FORMS,
		0,
		0,
		0,
		"A byte is read from the device identified by bc "
		"and stored in (hl). b is decremented and h is incremented. "
		"If bc!=0 inir repeats. inir consumes more t-states when it "
		"repeats.",
		"?1 ? ?1 "
	},
	{
		"jp",
		SPECASM_DOC_JP_FORMS,
		SPECASM_DOC_JP_NUM_FORMS,
		0,
		0,
		8,
		"Jump to the last operand if the condition is met or no "
		"condition is supplied. Instruction consumes fewer t-states "
		"if condition is not met.",
		NULL,
	},
	{
		"ldd",
		SPECASM_DOC_LDD_FORMS,
		SPECASM_DOC_LDD_NUM_FORMS,
		0,
		0,
		0,
		"Load (hl) into (de). hl, de and bc are decremented.",
		"    0 X0 ",
	},
	{
		"lddr",
		SPECASM_DOC_LDDR_FORMS,
		SPECASM_DOC_LDDR_NUM_FORMS,
		0,
		0,
		0,
		"Load (hl) into (de). hl, de and bc are decremented. If "
		"bc!=0 lddr repeats. lddr consumes more t-states when it "
		"repeats.",
		"   0 00 ",
	},
	{
		"ldi",
		SPECASM_DOC_LDI_FORMS,
		SPECASM_DOC_LDI_NUM_FORMS,
		0,
		0,
		0,
		"Load (hl) into (de). Both hl and de are incremented while bc "
		"is decremented.",
		"    0 X0 ",
	},
	{
		"ldir",
		SPECASM_DOC_LDIR_FORMS,
		SPECASM_DOC_LDIR_NUM_FORMS,
		0,
		0,
		0,
		"Load (hl) into (de). Both hl and de are incremented while bc "
		"is decremented. If bc!=0 ldir repeats. ldir "
		"consumes more t-states when it repeats.",
		"   0 00 ",
	},
	{
		"map",
		SPECASM_DOC_MAP_FORMS,
		SPECASM_DOC_MAP_NUM_FORMS,
		0,
		0,
		0,
		"Instructs the linker to generate a map file.",
		NULL,
	},
	{
		"neg",
		SPECASM_DOC_NEG_FORMS,
		SPECASM_DOC_NEG_NUM_FORMS,
		0,
		0,
		0,
		"Let a = 0 - a.",
		"XX X X1X",
	},
	{
		"nop",
		SPECASM_DOC_NOP_FORMS,
		SPECASM_DOC_NOP_NUM_FORMS,
		0,
		0,
		0,
		"CPU does nothing for 1 m-cycle.",
		NULL,
	},
	{
		"or",
		SPECASM_DOC_OR_FORMS,
		SPECASM_DOC_OR_NUM_FORMS,
		2,
		0,
		0,
		"The result of a bitwise OR of a and the argument is stored "
		"in a.",
		"XX 0 X00",
	},
	{
		"org",
		SPECASM_DOC_ORG_FORMS,
		SPECASM_DOC_ORG_NUM_FORMS,
		0,
		0,
		0,
		"Assembler directive that sets the org address of the program, "
		"i.e., the address the first byte in the .x or .t file that "
		"contains the Main label is assembled at.",
		NULL,
	},
	{
		"out",
		SPECASM_DOC_OUT1_FORMS,
		SPECASM_DOC_OUT1_NUM_FORMS,
		6,
		0,
		0,
		"Writes the register r to the device identified by bc.",
		NULL,
	},
	{
		"out",
		SPECASM_DOC_OUT2_FORMS,
		SPECASM_DOC_OUT2_NUM_FORMS,
		0,
		0,
		0,
		"Writes a to the device at the adddress whose MSB "
		"is taken from the a and whose LSB is n.",
		NULL,
	},
	{
		"outd",
		SPECASM_DOC_OUTD_FORMS,
		SPECASM_DOC_OUTD_NUM_FORMS,
		0,
		0,
		0,
		"b is decremented. (hl) is written to the device identified "
		"by  bc, where b has already been decremented. hl is "
		"decremented.",
		"?X ? ?1 ",
	},
	{
		"outdr",
		SPECASM_DOC_OUTDR_FORMS,
		SPECASM_DOC_OUTDR_NUM_FORMS,
		0,
		0,
		0,
		"b is decremented. (hl) is written to the device identified "
		"by bc, where b has already been decremented. hl is "
		"decremented. outdr repeats if b!=0. It consumes more t-states "
		"when it repeats.",
		"?1 ? ?1 ",
	},
	{
		"outi",
		SPECASM_DOC_OUTI_FORMS,
		SPECASM_DOC_OUTI_NUM_FORMS,
		0,
		0,
		0,
		"b is decremented. (hl) is written to the device identified by "
		"bc, where b has already been decremented. hl is incremented.",
		"?X ? ?1 ",
	},
	{
		"outir",
		SPECASM_DOC_OUTIR_FORMS,
		SPECASM_DOC_OUTIR_NUM_FORMS,
		0,
		0,
		0,
		"b is decremented. (hl) is written to the device identified by "
		"bc, where b has already been decremented. hl is incremented. "
		"outir repeats if b!=0. It consumes more t-states "
		"when it repeats.",
		"?1 ? ?1 ",
	},
	{
		"pop",
		SPECASM_DOC_POP_FORMS,
		SPECASM_DOC_POP_NUM_FORMS,
		7,
		0,
		0,
		"Pops 2 bytes off the stack into the operand.",
		NULL,
	},
	{
		"push",
		SPECASM_DOC_PUSH_FORMS,
		SPECASM_DOC_PUSH_NUM_FORMS,
		7,
		0,
		0,
		"Pushes the operand onto the stack.",
		NULL,
	},
	{
		"res",
		SPECASM_DOC_RES_FORMS,
		SPECASM_DOC_RES_NUM_FORMS,
		2,
		8,
		0,
		"Resets bit n in the second operand.",
		NULL,
	},
	{
		"ret",
		SPECASM_DOC_RET_FORMS,
		SPECASM_DOC_RET_NUM_FORMS,
		0,
		0,
		8,
		"If there is no condition or the condition is met, a word "
		"is popped off the stack into the PC, from which program "
		"execution continues. ret takes fewer cycles if the condition "
		"is not met.",
		NULL,
	},
	{
		"reti",
		SPECASM_DOC_RETI_FORMS,
		SPECASM_DOC_RETI_NUM_FORMS,
		0,
		0,
		0,
		"Return from interrupt. A word is popped off the stack into PC."
		" An ei must be executed prior to the reti to renable "
		"maskable interrupts.",
		NULL,
	},
	{
		"retn",
		SPECASM_DOC_RETN_FORMS,
		SPECASM_DOC_RETN_NUM_FORMS,
		0,
		0,
		0,
		"Return from NMI. A word is popped off the stack into PC and "
		"the maskable interrupts are re-enabled if they were enabled "
		"before the NMI.",
		NULL,
	},
	{
		"rl",
		SPECASM_DOC_RL_FORMS,
		SPECASM_DOC_RL_NUM_FORMS,
		2,
		0,
		0,
		"The operand is shifted left by 1 bit. The carry flag "
		"is moved into bit 0 of the operand and the old bit 7 of "
		"the operand is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"rla",
		SPECASM_DOC_RLA_FORMS,
		SPECASM_DOC_RLA_NUM_FORMS,
		0,
		0,
		0,
		"a is rotated left 1 bit. The carry flag is moved to bit 0 of "
		"a and its old bit 7 is moved to the carry flag.",
		"   0  0X",
	},
	{
		"rlc",
		SPECASM_DOC_RLC_FORMS,
		SPECASM_DOC_RLC_NUM_FORMS,
		2,
		0,
		0,
		"The operand is rotated left 1 bit. The old bit 7 is moved to "
		"both the carry flag and bit 0 of the operand.",
		"XX 0 X0X",
	},
	{
		"rlca",
		SPECASM_DOC_RLCA_FORMS,
		SPECASM_DOC_RLCA_NUM_FORMS,
		0,
		0,
		0,
		"a is rotated left 1 bit. The old bit 7 is moved "
		"to both the carry flag and bit 0 of a.",
		"   0  0X",
	},
	{
		"rld",
		SPECASM_DOC_RLD_FORMS,
		SPECASM_DOC_RLD_NUM_FORMS,
		0,
		0,
		0,
		"Let tmp = (hl) >> 4             "
		"Let (hl) = ((hl) << 4)|(a & $f) "
		"Let a = (a & $f0)|tmp",
		"XX 0 X0 ",
	},
	{
		"rr",
		SPECASM_DOC_RR_FORMS,
		SPECASM_DOC_RR_NUM_FORMS,
		2,
		0,
		0,
		"The operand is shifted right by 1 bit. The carry flag "
		"is moved into bit 7 of the operand and the old bit 0 of "
		"the operand is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"rra",
		SPECASM_DOC_RRA_FORMS,
		SPECASM_DOC_RRA_NUM_FORMS,
		0,
		0,
		0,
		"a is shifted right 1 bit. The carry flag is "
		"moved into bit 7 of a and the old contents of a's "
		"bit 0 are moved into the carry flag.",
		"   0  0X",
	},
	{
		"rrc",
		SPECASM_DOC_RRC_FORMS,
		SPECASM_DOC_RRC_NUM_FORMS,
		2,
		0,
		0,
		"The operand is rotated right by 1 bit. Its old bit 0 is "
		"moved into both the carry flag and bit 7 of the operand.",
		"XX 0 X0X",
	},
	{
		"rrca",
		SPECASM_DOC_RRCA_FORMS,
		SPECASM_DOC_RRCA_NUM_FORMS,
		0,
		0,
		0,
		"a is rotated right by 1 bit. a's old bit 0 is "
		"moved into both the carry flag and bit 7 of a.",
		"   0  0X",
	},
	{
		"rrd",
		SPECASM_DOC_RRD_FORMS,
		SPECASM_DOC_RRD_NUM_FORMS,
		0,
		0,
		0,
		"Let tmp = a << 4             "
		"Let a = (a & $f0)|((hl) & $f) "
		"Let (hl) = tmp | ((hl) >> 4)",
		"XX 0 X0 ",
	},
	{
		"rst",
		SPECASM_DOC_RST_FORMS,
		SPECASM_DOC_RST_NUM_FORMS,
		0,
		0,
		0,
		"The PC is pushed to the stack and the CPU jumps to the "
		"address n.  Valid values of n are 0, 8, 16, 24, 32, 40, 48,"
		" and 56.",
		NULL,
	},
	{
		"sbc",
		SPECASM_DOC_SBC_FORMS,
		SPECASM_DOC_SBC_NUM_FORMS,
		1,
		0,
		0,
		"The second argument and the carry flag are subtracted from "
		"the contents of the destination register, either a or hl.",
		"XX X X1X",
	},
	{
		"scf",
		SPECASM_DOC_SCF_FORMS,
		SPECASM_DOC_SCF_NUM_FORMS,
		0,
		0,
		0,
		"Sets the carry flag.",
		"   0  01"
	},
	{
		"set",
		SPECASM_DOC_SET_FORMS,
		SPECASM_DOC_SET_NUM_FORMS,
		2,
		8,
		0,
		"Sets bit n in the second operand.",
		NULL,
	},
	{
		"sla",
		SPECASM_DOC_SLA_FORMS,
		SPECASM_DOC_SLA_NUM_FORMS,
		2,
		0,
		0,
		"The operand is shifted left by 1 bit. Bit 0 "
		"is set to 0. Bit 7 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"sra",
		SPECASM_DOC_SRA_FORMS,
		SPECASM_DOC_SRA_NUM_FORMS,
		2,
		0,
		0,
		"The operand is arithmetically shifted right by 1 bit. Bit 7 "
		"remains unchanged. Bit 0 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"srl",
		SPECASM_DOC_SRL_FORMS,
		SPECASM_DOC_SRL_NUM_FORMS,
		2,
		0,
		0,
		"The operand is logically shifted right by 1 bit. Bit 7 is "
		"set to 0. Bit 0 is moved into the carry flag.",
		"XX 0 X0X",
	},
	{
		"sub",
		SPECASM_DOC_SUB_FORMS,
		SPECASM_DOC_SUB_NUM_FORMS,
		2,
		0,
		0,
		"The operand is subracted from a.",
		"XX X X1X",
	},
	{
		"xor",
		SPECASM_DOC_XOR_FORMS,
		SPECASM_DOC_XOR_NUM_FORMS,
		2,
		0,
		0,
		"The result of a bitwise XOR of a and the argument is stored "
		"in a.",
		"XX 0 X00",
	},
	{
		"zx81",
		SPECASM_DOC_MAP_FORMS,
		SPECASM_DOC_MAP_NUM_FORMS,
		0,
		0,
		0,
		"Linker directive that causes string and character literals to "
		"be transliterated from ASCII to ZX81 encoding.  It also sets "
		"org address to 16514.",
		NULL,
	},

};

const uint8_t max_docs = sizeof(docs) / sizeof(specasm_ins_doc_t);

static uint8_t prv_pretty_print(uint8_t y, const char * text)
{
	uint8_t i;

	while (*text) {
		memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
		scratch[SPECASM_LINE_MAX_LEN] = 0;
		for (i = 0; *text && i < SPECASM_LINE_MAX_LEN; i++) {
			scratch[i] = *text++;
		}

		if (i == SPECASM_LINE_MAX_LEN) {
			for (i = i - 1; scratch[i] != ' '; i--) {
				text--;
				scratch[i] = ' ';
				if (i == 0)
					break;
			}
		}

		specasm_util_print(scratch, 0, y++, PAPER_BLACK | INK_WHITE);
	}

	return y;
}

static void prv_justify_num(uint8_t num)
{
	char *s = scratch;

	if (num < 10) {
		s[0] = ' ';
		s++;
	}
	itoa(num, s, 10);
}

static uint8_t prv_print_register_encoding(const specasm_ins_doc_t *doc,
					   uint8_t y)
{
	uint8_t i;
	uint8_t reg_encoding;
	const char* reg_name;
	char *s2;
	char *rr_start = NULL;
	const uint8_t* enc;
	char *s = scratch;

	if (!doc->reg_encoding)
		return y;

	enc = (const uint8_t*) &reg_encodings[doc->reg_encoding-1];

	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			if ((i > 6) && !rr_start)
				rr_start = s;
			s += 3;
		}
	}

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	if (!rr_start) {
		scratch[15] = 'r';
	} else if (rr_start == scratch) {
		scratch[14] = 'r';
		scratch[15] = 'r';
	} else {
		scratch[(rr_start - scratch) / 2] = 'r';
		s2 = SPECASM_LINE_MAX_LEN + scratch;
		s = &rr_start[((s2 - rr_start) / 2)];
		s -= 3;
		*s = 'r';
		s[1] = 'r';
	}
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	s = scratch;
	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			s2 = s;
			reg_name = reg_names[i];
			if (!reg_name[1])
				s2++;
			memcpy(s2, reg_name, strlen(reg_name));
			s +=3;
		}
	}
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	s = scratch;
	for (i = 0; i < SPECASM_DOC_MAX_REG_ENCODING; i++) {
		reg_encoding = enc[i];
		if (reg_encoding) {
			reg_encoding--;
			s2 = s;
			if (reg_encoding < 16)
				s2++;
			itoa(reg_encoding, s2, 16);
			s2[strlen(s2)] = ' ';
			s +=3;
		}
	}

	specasm_util_print(scratch, 0, y + 1, PAPER_BLACK | INK_WHITE | 64);

	return y + 3;
}

static void prv_print_flags(uint8_t y, const char *flags)
{
	if (!flags)
		return;

	specasm_util_print("sz-h-pnc", 12, y, PAPER_BLUE | INK_WHITE);
	y++;
	specasm_util_print("Flags", 5, y, PAPER_BLACK | INK_WHITE);
	specasm_util_print(flags, 12, y, PAPER_BLACK | INK_WHITE | 64);
}

static uint8_t prv_print_bits_encoding(const specasm_ins_doc_t *doc,
				       uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	char *s;

	if (!doc->bits)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[15] = 'b';
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	s = scratch;
	for (i = 0; i < 8; i++) {
		s[2] = i + '0';
		s += 4;
	}
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);
	y++;

	for (i = 0; i < 8; i++) {
		num = doc->bits * i;
		itoa(num, scratch, 16);
		x = (i*4)+1;
		if (num < 16)
			x++;
		specasm_util_print(scratch, x, y, PAPER_BLACK | INK_WHITE);
	}

	return y + 2;
}

static uint8_t prv_print_cc_encoding(const specasm_ins_doc_t *doc,
				     uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	char *s;

	if (!doc->all_cc)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[14] = 'c';
	scratch[15] = 'c';
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	specasm_util_print(" nz   z  nc   c  po  pe   p   m ", 0,
			   y, PAPER_BLUE | INK_WHITE);
	y++;

	for (i = 0; i < 8; i++) {
		num = doc->all_cc * i;
		itoa(num, scratch, 16);
		x = (i*4)+1;
		if (num < 16)
			x++;
		specasm_util_print(scratch, x, y, PAPER_BLACK | INK_WHITE);
	}

	return y + 2;
}

static void prv_draw_help(uint8_t ins_id)
{
	uint8_t x;
	uint8_t i;
	uint8_t y;
	uint8_t col;
	const specasm_ins_doc_t* doc = &docs[ins_id];
	const specasm_ins_form_t *form;
	const char* ins_name = doc->name;
	uint8_t ins_name_len = strlen(ins_name);

	specasm_cls(PAPER_BLACK | INK_WHITE);
	x = (SPECASM_LINE_MAX_LEN / 2) - (strlen(ins_name) >> 1);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	memcpy(&scratch[x], ins_name, ins_name_len);

	specasm_util_print(scratch, 0, 0, PAPER_BLUE | INK_WHITE | 64);

	y = 2;
	memcpy(scratch, "Opcode          Encoding    M  T",
	       SPECASM_LINE_MAX_LEN);
	specasm_util_print(scratch, 0, y, PAPER_BLUE | INK_WHITE);
	y++;
	col = PAPER_BLACK | INK_WHITE;
	form = &specasm_forms[doc->forms];
	for (i = 0; i < doc->num_forms; i++) {
		col ^= 1 << 6;
		x = 0;
		specasm_util_print(ins_name, x, y, col);
		x += ins_name_len + 1;
		specasm_util_print(form->form, x, y, col);
		specasm_util_print(form->encoding, SPECASM_DOC_ENCODING_X, y,
				   col);
		prv_justify_num(form->m_cycles);
		specasm_util_print(scratch, SPECASM_DOC_M_CYCLES_X, y, col);
		prv_justify_num(form->t_states);
		specasm_util_print(scratch, SPECASM_DOC_T_STATES_X, y, col);
		y++;
		form++;
	}

	y = prv_print_register_encoding(doc, y + 1);
	y = prv_print_bits_encoding(doc, y);
	y = prv_print_cc_encoding(doc, y);
	y = prv_pretty_print(y, doc->description);
	prv_print_flags(y + 1, doc->flags);
}

static uint8_t prv_find_mnemomic(const char *ins_name)
{
	uint8_t m;
	uint8_t l;
	uint8_t r;
	int res;

	if (strcmp(ins_name, docs[0].name) < 0)
		return 0;

	l = 0;
	r =max_docs - 1;

	while (l <= r) {
		m = (l + r) >> 1;
		res = strcmp(docs[m].name, ins_name);
		if (res < 0) {
			l = m + 1;
		} else if (res > 0) {
			if (m == 0)
				break;
			r = m - 1;
		} else {
			return m;
		}
	}

	/*
	 * Opcode not found.  Let's return something close.
	 */

	return r;
}

void specasm_help_banked(const char *ins_name)
{
	uint8_t id;
	uint8_t k;

	id = prv_find_mnemomic(ins_name);
	do {
		prv_draw_help(id);
		do {
			specasm_sleep_ms(25);
		} while(!(k = in_inkey()));

		if (k == SPECASM_KEY_LEFT) {
			if (id > 0)
				id--;
		} else if (k == SPECASM_KEY_RIGHT) {
			if (id < max_docs - 1)
				id++;
		} else {
			break;
		}
	} while(1);

	specasm_cls(PAPER_BLACK | INK_WHITE);
}
