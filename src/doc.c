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

#include <input.h>
#include <stdlib.h>
#include <z80.h>

#include "descra2l.h"
#include "descrm2z.h"
#include "doc.h"
#include "editor.h"
#include "line_common.h"
#include "peer.h"
#include "scratch.h"

#include <string.h>

#ifdef SPECASM_NEXT_BANKED
void specasm_descr_bank(const char *ins_name);
#endif

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

/* clang-format off */

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

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_ADD5_FORMS \
	(SPECASM_DOC_ADD4_FORMS + SPECASM_DOC_ADD4_NUM_FORMS)
#define SPECASM_DOC_ADD5_NUM_FORMS 1
#define SPECASM_DOC_ADD6_FORMS \
	(SPECASM_DOC_ADD5_FORMS + SPECASM_DOC_ADD5_NUM_FORMS)
#define SPECASM_DOC_ADD6_NUM_FORMS 1
#define SPECASM_DOC_ALIGN_FORMS \
	(SPECASM_DOC_ADD6_FORMS + SPECASM_DOC_ADD6_NUM_FORMS)
#else
#define SPECASM_DOC_ALIGN_FORMS \
	(SPECASM_DOC_ADD4_FORMS + SPECASM_DOC_ADD4_NUM_FORMS)
#endif

#define SPECASM_DOC_ALIGN_NUM_FORMS 1
#define SPECASM_DOC_AND_FORMS \
	(SPECASM_DOC_ALIGN_FORMS + SPECASM_DOC_ALIGN_NUM_FORMS)
#define SPECASM_DOC_AND_NUM_FORMS 5
#define SPECASM_DOC_BIT_FORMS \
	(SPECASM_DOC_AND_FORMS + SPECASM_DOC_AND_NUM_FORMS)
#define SPECASM_DOC_BIT_NUM_FORMS 4

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_BRLC_FORMS \
	(SPECASM_DOC_BIT_FORMS + SPECASM_DOC_BIT_NUM_FORMS)
#define SPECASM_DOC_BRLC_NUM_FORMS 1
#define SPECASM_DOC_BSLA_FORMS \
	(SPECASM_DOC_BRLC_FORMS + SPECASM_DOC_BRLC_NUM_FORMS)
#define SPECASM_DOC_BSLA_NUM_FORMS 1
#define SPECASM_DOC_BSRA_FORMS \
	(SPECASM_DOC_BSLA_FORMS + SPECASM_DOC_BSLA_NUM_FORMS)
#define SPECASM_DOC_BSRA_NUM_FORMS 1
#define SPECASM_DOC_BSRF_FORMS \
	(SPECASM_DOC_BSRA_FORMS + SPECASM_DOC_BSRA_NUM_FORMS)
#define SPECASM_DOC_BSRF_NUM_FORMS 1
#define SPECASM_DOC_BSRL_FORMS \
	(SPECASM_DOC_BSRF_FORMS + SPECASM_DOC_BSRF_NUM_FORMS)
#define SPECASM_DOC_BSRL_NUM_FORMS 1
#define SPECASM_DOC_CALL_FORMS \
	(SPECASM_DOC_BSRL_FORMS + SPECASM_DOC_BSRL_NUM_FORMS)
#else
#define SPECASM_DOC_CALL_FORMS \
	(SPECASM_DOC_BIT_FORMS + SPECASM_DOC_BIT_NUM_FORMS)
#endif

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

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_JP_C_FORMS \
	(SPECASM_DOC_JP_FORMS + SPECASM_DOC_JP_NUM_FORMS)
#define SPECASM_DOC_JP_C_NUM_FORMS 1
#define SPECASM_DOC_JR_FORMS \
	(SPECASM_DOC_JP_C_FORMS + SPECASM_DOC_JP_C_NUM_FORMS)
#else
#define SPECASM_DOC_JR_FORMS \
	(SPECASM_DOC_JP_FORMS + SPECASM_DOC_JP_NUM_FORMS)
#endif

#define SPECASM_DOC_JR_NUM_FORMS 3
#define SPECASM_DOC_LD1_FORMS \
	(SPECASM_DOC_JR_FORMS + SPECASM_DOC_JR_NUM_FORMS)
#define SPECASM_DOC_LD1_NUM_FORMS 7
#define SPECASM_DOC_LD2_FORMS \
	(SPECASM_DOC_LD1_FORMS + SPECASM_DOC_LD1_NUM_FORMS)
#define SPECASM_DOC_LD2_NUM_FORMS 3
#define SPECASM_DOC_LD3_FORMS \
	(SPECASM_DOC_LD2_FORMS + SPECASM_DOC_LD2_NUM_FORMS)
#define SPECASM_DOC_LD3_NUM_FORMS 2
#define SPECASM_DOC_LD4_FORMS \
	(SPECASM_DOC_LD3_FORMS + SPECASM_DOC_LD3_NUM_FORMS)
#define SPECASM_DOC_LD4_NUM_FORMS 6
#define SPECASM_DOC_LD5_FORMS \
	(SPECASM_DOC_LD4_FORMS + SPECASM_DOC_LD4_NUM_FORMS)
#define SPECASM_DOC_LD5_NUM_FORMS 4
#define SPECASM_DOC_LD6_FORMS \
	(SPECASM_DOC_LD5_FORMS + SPECASM_DOC_LD5_NUM_FORMS)
#define SPECASM_DOC_LD6_NUM_FORMS 3
#define SPECASM_DOC_LD7_FORMS \
	(SPECASM_DOC_LD6_FORMS + SPECASM_DOC_LD6_NUM_FORMS)
#define SPECASM_DOC_LD7_NUM_FORMS 5
#define SPECASM_DOC_LD8_FORMS \
	(SPECASM_DOC_LD7_FORMS + SPECASM_DOC_LD7_NUM_FORMS)
#define SPECASM_DOC_LD8_NUM_FORMS 4
#define SPECASM_DOC_LDD_FORMS \
	(SPECASM_DOC_LD8_FORMS + SPECASM_DOC_LD8_NUM_FORMS)
#define SPECASM_DOC_LDD_NUM_FORMS 1
#define SPECASM_DOC_LDDR_FORMS \
	(SPECASM_DOC_LDD_FORMS + SPECASM_DOC_LDD_NUM_FORMS)
#define SPECASM_DOC_LDDR_NUM_FORMS 2

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_LDDRX_FORMS \
	(SPECASM_DOC_LDDR_FORMS + SPECASM_DOC_LDDR_NUM_FORMS)
#define SPECASM_DOC_LDDRX_NUM_FORMS 2
#define SPECASM_DOC_LDDX_FORMS \
	(SPECASM_DOC_LDDRX_FORMS + SPECASM_DOC_LDDRX_NUM_FORMS)
#define SPECASM_DOC_LDDX_NUM_FORMS 1
#define SPECASM_DOC_LDI_FORMS \
	(SPECASM_DOC_LDDX_FORMS + SPECASM_DOC_LDDX_NUM_FORMS)
#else
#define SPECASM_DOC_LDI_FORMS \
	(SPECASM_DOC_LDDR_FORMS + SPECASM_DOC_LDDR_NUM_FORMS)
#endif

#define SPECASM_DOC_LDI_NUM_FORMS 1
#define SPECASM_DOC_LDIR_FORMS \
	(SPECASM_DOC_LDI_FORMS + SPECASM_DOC_LDI_NUM_FORMS)
#define SPECASM_DOC_LDIR_NUM_FORMS 2

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_LDIRX_FORMS \
	(SPECASM_DOC_LDIR_FORMS + SPECASM_DOC_LDIR_NUM_FORMS)
#define SPECASM_DOC_LDIRX_NUM_FORMS 2
#define SPECASM_DOC_LDIX_FORMS \
	(SPECASM_DOC_LDIRX_FORMS + SPECASM_DOC_LDIRX_NUM_FORMS)
#define SPECASM_DOC_LDIX_NUM_FORMS 1
#define SPECASM_DOC_LDPIRX_FORMS \
	(SPECASM_DOC_LDIX_FORMS + SPECASM_DOC_LDIX_NUM_FORMS)
#define SPECASM_DOC_LDPIRX_NUM_FORMS 2
#define SPECASM_DOC_LDWS_FORMS \
	(SPECASM_DOC_LDPIRX_FORMS + SPECASM_DOC_LDPIRX_NUM_FORMS)
#define SPECASM_DOC_LDWS_NUM_FORMS 1
#define SPECASM_DOC_MAP_FORMS \
	(SPECASM_DOC_LDWS_FORMS + SPECASM_DOC_LDWS_NUM_FORMS)
#define SPECASM_DOC_MAP_NUM_FORMS 1
#define SPECASM_DOC_MIRROR_FORMS \
	(SPECASM_DOC_MAP_FORMS + SPECASM_DOC_MAP_NUM_FORMS)
#define SPECASM_DOC_MIRROR_NUM_FORMS 1
#define SPECASM_DOC_MUL_FORMS \
	(SPECASM_DOC_MIRROR_FORMS + SPECASM_DOC_MIRROR_NUM_FORMS)
#define SPECASM_DOC_MUL_NUM_FORMS 1
#define SPECASM_DOC_NEG_FORMS \
	(SPECASM_DOC_MUL_FORMS + SPECASM_DOC_MUL_NUM_FORMS)
#define SPECASM_DOC_NEG_NUM_FORMS 1
#define SPECASM_DOC_NEXTREG_FORMS \
	(SPECASM_DOC_NEG_FORMS + SPECASM_DOC_NEG_NUM_FORMS)
#define SPECASM_DOC_NEXTREG_NUM_FORMS 2
#define SPECASM_DOC_NOP_FORMS \
	(SPECASM_DOC_NEXTREG_FORMS + SPECASM_DOC_NEXTREG_NUM_FORMS)
#else
#define SPECASM_DOC_MAP_FORMS \
	(SPECASM_DOC_LDIR_FORMS + SPECASM_DOC_LDIR_NUM_FORMS)
#define SPECASM_DOC_MAP_NUM_FORMS 1
#define SPECASM_DOC_NEG_FORMS \
	(SPECASM_DOC_MAP_FORMS + SPECASM_DOC_MAP_NUM_FORMS)
#define SPECASM_DOC_NEG_NUM_FORMS 1
#define SPECASM_DOC_NOP_FORMS \
	(SPECASM_DOC_NEG_FORMS + SPECASM_DOC_NEG_NUM_FORMS)
#endif

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

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_OUTINB_FORMS \
	(SPECASM_DOC_OUTI_FORMS + SPECASM_DOC_OUTI_NUM_FORMS)
#define SPECASM_DOC_OUTINB_NUM_FORMS 1
#define SPECASM_DOC_OUTIR_FORMS \
	(SPECASM_DOC_OUTINB_FORMS + SPECASM_DOC_OUTINB_NUM_FORMS)
#define SPECASM_DOC_OUTIR_NUM_FORMS 2

#define SPECASM_DOC_PIXELAD_FORMS \
	(SPECASM_DOC_OUTIR_FORMS + SPECASM_DOC_OUTIR_NUM_FORMS)
#define SPECASM_DOC_PIXELAD_NUM_FORMS 1
#define SPECASM_DOC_PIXELDN_FORMS \
	(SPECASM_DOC_PIXELAD_FORMS + SPECASM_DOC_PIXELAD_NUM_FORMS)
#define SPECASM_DOC_PIXELDN_NUM_FORMS 1
#define SPECASM_DOC_POP_FORMS \
	(SPECASM_DOC_PIXELDN_FORMS + SPECASM_DOC_PIXELDN_NUM_FORMS)
#else
#define SPECASM_DOC_OUTIR_FORMS \
	(SPECASM_DOC_OUTI_FORMS + SPECASM_DOC_OUTI_NUM_FORMS)
#define SPECASM_DOC_OUTIR_NUM_FORMS 2
#define SPECASM_DOC_POP_FORMS \
	(SPECASM_DOC_OUTIR_FORMS + SPECASM_DOC_OUTIR_NUM_FORMS)
#endif

#define SPECASM_DOC_POP_NUM_FORMS 3
#define SPECASM_DOC_PUSH_FORMS \
	(SPECASM_DOC_POP_FORMS + SPECASM_DOC_POP_NUM_FORMS)
#define SPECASM_DOC_PUSH_NUM_FORMS 3

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_PUSH_IMM_FORMS \
	(SPECASM_DOC_PUSH_FORMS + SPECASM_DOC_PUSH_NUM_FORMS)
#define SPECASM_DOC_PUSH_IMM_NUM_FORMS 1
#define SPECASM_DOC_RES_FORMS \
	(SPECASM_DOC_PUSH_IMM_FORMS + SPECASM_DOC_PUSH_IMM_NUM_FORMS)
#else
#define SPECASM_DOC_RES_FORMS \
	(SPECASM_DOC_PUSH_FORMS + SPECASM_DOC_PUSH_NUM_FORMS)
#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_SETAE_FORMS \
	(SPECASM_DOC_SET_FORMS + SPECASM_DOC_SET_NUM_FORMS)
#define SPECASM_DOC_SETAE_NUM_FORMS 1
#define SPECASM_DOC_SLA_FORMS \
	(SPECASM_DOC_SETAE_FORMS + SPECASM_DOC_SETAE_NUM_FORMS)
#else
#define SPECASM_DOC_SLA_FORMS \
	(SPECASM_DOC_SET_FORMS + SPECASM_DOC_SET_NUM_FORMS)
#endif

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

#ifdef SPECASM_TARGET_NEXT_OPCODES
#define SPECASM_DOC_SWAPNIB_FORMS \
	(SPECASM_DOC_SUB_FORMS + SPECASM_DOC_SUB_NUM_FORMS)
#define SPECASM_DOC_SWAPNIB_NUM_FORMS 1
#define SPECASM_DOC_TEST_FORMS \
	(SPECASM_DOC_SWAPNIB_FORMS + SPECASM_DOC_SWAPNIB_NUM_FORMS)
#define SPECASM_DOC_TEST_NUM_FORMS 1
#define SPECASM_DOC_XOR_FORMS \
	(SPECASM_DOC_TEST_FORMS + SPECASM_DOC_TEST_NUM_FORMS)
#else
#define SPECASM_DOC_XOR_FORMS \
	(SPECASM_DOC_SUB_FORMS + SPECASM_DOC_SUB_NUM_FORMS)

#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_ADD5_FORMS */
	{"rr,a", "ED 30+rr", 2, 8 },

	/* SPECASM_DOC_ADD6_FORMS */
	{"rr,nn", "ED 34+nn", 4, 16 },
#endif
	/* SPECASM_DOC_ALIGN_FORMS */
	{"n", "0", 1, 4 },

	/* SPECASM_DOC_AND_FORMS */
	{"r", "A0+r", 1, 4},
	{"n", "E6 n",  2, 7},
	{"(hl)", "A6", 2, 7},
	{"(ix+d)", "DD A6 d", 5, 19 },
	{"(iy+d)", "FD A6 d", 5, 19 },

	/* SPECASM_DOC_BIT_FORMS */
	{"n,r", "CB 40+n+r", 2, 8},
	{"n,(hl)", "CB 46+n", 3, 12},
	{"n,(ix+d)", "DDCBd46+n", 5, 20},
	{"n,(iy+d)", "FDCBd46+n", 5, 20},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_BRLC_FORMS */
	{"de,b", "ED 2C", 2, 8},

	/* SPECASM_DOC_BSLA_FORMS */
	{"de,b", "ED 28", 2, 8},

	/* SPECASM_DOC_BSRA_FORMS */
	{"de,b", "ED 29", 2, 8},

	/* SPECASM_DOC_BSRF_FORMS */
	{"de,b", "ED 2B", 2, 8},

	/* SPECASM_DOC_BSRL_FORMS */
	{"de,b", "ED 2A", 2, 8},
#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_JP_C_FORMS */
	{"(c)", "ED 98", 3, 13},
#endif

	/* SPECASM_DOC_JR_FORMS */
	{"n", "18 n", 3, 12},
	{"cc,n", "40+cc n", 3, 12},
	{"cc,n", "40+cc n", 2, 7},

	/* SPECASM_DOC_LD1_FORMS */
	{"rr,nn", "1+rr n n", 3, 10},
	{"ix,nn", "DD 21 n n", 4, 14},
	{"iy,nn", "FD 21 n n", 4, 14},
	{"hl,(nn)","2A n n", 5, 16},
	{"rr,(nn)", "ED 4A+rr", 6, 20},
	{"ix,(nn)", "DD 2A n n", 6, 20},
	{"iy,(nn)", "FD 2A n n", 6, 20},

	/* SPECASM_DOC_LD2_FORMS */
	{"sp,hl", "F9", 1, 6},
	{"sp,ix", "DD F9", 2, 10},
	{"sp,iy", "FD F9", 2, 10},

	/* SPECASM_DOC_LD3_FORMS */
	{"r,n", "6+r n", 2, 7},
	{"r,r'", "40+r+(r'/8)", 1, 4},

	/* SPECASM_DOC_LD4_FORMS */
	{"(bc),a", "02", 2, 7},
	{"(de),a", "12", 2, 7},
	{"(hl),r", "70+r", 2, 7},
	{"(ix+d),r", "DD 70+r d", 5, 19},
	{"(iy+d),r", "FD 70+r d", 5, 19},
	{"(nn),a", "32 n n", 4, 13},

	/* SPECASM_DOC_LD5_FORMS */
	{"(nn), hl", "22 nn", 5, 16},
	{"(nn), rr", "ED 43+rr nn", 6, 20},
	{"(nn), ix", "DD 22 nn", 6, 20},
	{"(nn), iy", "FD 22 nn", 6, 20},

	/* SPECASM_DOC_LD6_FORMS */
	{"(hl),n", "36 n", 3, 10},
	{"(ix+d),n", "DD 36 d n", 5, 19},
	{"(iy+d),n", "FD 36 d n", 5, 19},

	/* SPECASM_DOC_LD7_FORMS */
	{"r,(hl)", "46+r", 2, 7},
	{"a,(bc)", "0A", 2, 7},
	{"a,(de)", "1A", 2, 7},
	{"r,(ix+d)","DD 46 d", 5, 19},
	{"r,(iy+d)","FD 46 d", 5, 19},

	/* SPECASM_DOC_LD8_FORMS */
	{"a,i", "ED 57", 2, 9},
	{"a,r", "ED 5F", 2, 9},
	{"i,a", "ED 47", 2, 9},
	{"r,a", "ED 4F", 2, 9},

	/* SPECASM_DOC_LDD_FORMS */
	{"", "ED A8", 4, 16},

	/* SPECASM_DOC_LDDR_FORMS */
	{"", "ED B8", 4, 16},
	{"", "ED B8", 5, 21},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_LDDRX_FORMS */
	{"", "ED BC", 4, 16},
	{"", "ED BC", 5, 21},
	
	/* SPECASM_DOC_LDDX_FORMS */
	{"", "ED AC", 4, 16},
#endif

	/* SPECASM_DOC_LDI_FORMS */
	{"", "ED A0", 4, 16},

	/* SPECASM_DOC_LDIR_FORMS */
	{"", "ED B0", 4, 16},
	{"", "ED B0", 5, 21},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_LDIRX_FORMS */
	{"", "ED B4", 4, 16},
	{"", "ED B4", 5, 21},
	
	/* SPECASM_DOC_LDIX_FORMS */
	{"", "ED A4", 4, 16},

	/* SPECASM_DOC_LDPIRX_FORMS */
	{"", "ED B7", 4, 16},
	{"", "ED B7", 5, 21},
	
	/* SPECASM_DOC_LDWS_FORMS */
	{"", "ED A5", 4, 16},
#endif
	/* SPECASM_DOC_MAP_FORMS */
	{"", "", 0, 0},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_MIRROR_FORMS */
	{"a", "ED 24", 2, 8},
	
	/* SPECASM_DOC_MUL_FORMS */
	{"d,e", "ED 30", 2, 8},
#endif

	/* SPECASM_DOC_NEG_FORMS */
	{"", "ED 44", 2, 8},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_NEXTREG_FORMS */
	{"n,a", "ED 92 n", 4, 17},
	{"n,m", "ED 91 n m", 5, 20},
#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_OUTINB_FORMS */
	{"","ED 90", 4, 16},
#endif
	/* SPECASM_DOC_OUTIR_FORMS */
	{"","ED B3", 4, 16},
	{"","ED B3", 5, 21},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_PIXELAD_FORMS */
	{"","ED 94", 2, 8},

	/* SPECASM_DOC_PIXELDN_FORMS */
	{"","ED 93", 2, 8},
#endif
	/* SPECASM_DOC_POP_FORMS */
	{"rr", "C1+rr", 3, 10},
	{"ix", "DD E1", 4, 14},
	{"iy", "FD E1", 4, 14},

	/* SPECASM_DOC_PUSH_FORMS */
	{"rr", "C5+rr", 3, 11},
	{"ix", "DD E5", 4, 15},
	{"iy", "FD E5", 4, 15},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_PUSH_IMM_FORMS */
	{"nm", "ED 8A+nm", 6, 23},
#endif
	/* SPECASM_DOC_RES_FORMS */
	{"n,r", "CB 80+b+r", 2, 8},
	{"n,(hl)", "CB 86+n", 4, 15},
	{"n,(ix+d)", "DDCBd86+n", 6, 23},
	{"n,(iy+d)", "FDCBd86+n", 6, 23},

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
	{"n,(hl)", "CB C6+n", 4, 15},
	{"n,(ix+d)", "DDCBdC6+n", 6, 23},
	{"n,(iy+d)", "FDCBdC6+n", 6, 23},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_SETAE_FORMS */
	{"", "ED 95", 2, 8},
#endif
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

#ifdef SPECASM_TARGET_NEXT_OPCODES
	/* SPECASM_DOC_SWAPNIB_FORMS */
	{"", "ED 23", 2, 8},

	/* SPECASM_DOC_TEST_FORMS */

	{"n", "ED 27 n", 3, 11},
#endif

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
#ifdef SPECASM_TARGET_NEXT_OPCODES
	uint8_t zxn;
#endif
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
	{ 0, 0, 0, 0, 0, 0, 0, 4, 3, 2, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 3, 2, 1, 0 },
};

/*
 * Must be in the same order as the opcode_table in line_parse.c.
 */

const static specasm_ins_doc_t docs[] = {
	{
		"adc",
		SPECASM_DOC_ADC_FORMS,
		SPECASM_DOC_ADC_NUM_FORMS,
		1,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_adc,
		"XX X X0X",
	},
	{
		"add",
		SPECASM_DOC_ADD1_FORMS,
		SPECASM_DOC_ADD1_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_add,
		"XX X X0X",
	},
	{
		"add",
		SPECASM_DOC_ADD2_FORMS,
		SPECASM_DOC_ADD2_NUM_FORMS,
		3,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_add_16,
		"   X  0X",
	},
	#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"add",
		SPECASM_DOC_ADD5_FORMS,
		SPECASM_DOC_ADD5_NUM_FORMS,
		8,
		0,
		0,
		1,
		specasm_doc_add_rra,
		"       ?",
	},
	{
		"add",
		SPECASM_DOC_ADD6_FORMS,
		SPECASM_DOC_ADD6_NUM_FORMS,
		9,
		0,
		0,
		1,
		specasm_doc_add_16imm,
		NULL,
	},
	#endif
	{
		"align",
		SPECASM_DOC_ALIGN_FORMS,
		SPECASM_DOC_ALIGN_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_align,
		NULL,
	},
	{
		"and",
		SPECASM_DOC_AND_FORMS,
		SPECASM_DOC_AND_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_and,
		"XX 1 X00",
	},
	{
		"bit",
		SPECASM_DOC_BIT_FORMS,
		SPECASM_DOC_BIT_NUM_FORMS,
		2,
		8,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_bit,
		"?X 1 ?0 ",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"brlc",
		SPECASM_DOC_BRLC_FORMS,
		SPECASM_DOC_BRLC_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_brlc,
		NULL,
	},
	{
		"bsla",
		SPECASM_DOC_BSLA_FORMS,
		SPECASM_DOC_BSLA_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_bsla,
		NULL,
	},
	{
		"bsra",
		SPECASM_DOC_BSRA_FORMS,
		SPECASM_DOC_BSRA_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_bsra,
		NULL,
	},
	{
		"bsrf",
		SPECASM_DOC_BSRF_FORMS,
		SPECASM_DOC_BSRF_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_bsrf,
		NULL,
	},
	{
		"bsrl",
		SPECASM_DOC_BSRL_FORMS,
		SPECASM_DOC_BSRL_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_bsrl,
		NULL,
	},

#endif
	{
		"call",
		SPECASM_DOC_CALL_FORMS,
		SPECASM_DOC_CALL_NUM_FORMS,
		0,
		0,
		8,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_call,
		NULL,
	},
	{
		"ccf",
		SPECASM_DOC_CCF_FORMS,
		SPECASM_DOC_CCF_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ccf,
		"   ?  0X"
	},
	{
		"cp",
		SPECASM_DOC_CP_FORMS,
		SPECASM_DOC_CP_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cp,
		"XX X X1X",
	},
	{
		"cpd",
		SPECASM_DOC_CPD_FORMS,
		SPECASM_DOC_CPD_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cpd,
		"XX X X1 ",
	},
	{
		"cpdr",
		SPECASM_DOC_CPDR_FORMS,
		SPECASM_DOC_CPDR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cpdr,
		"XX X X1 ",
	},
	{
		"cpi",
		SPECASM_DOC_CPI_FORMS,
		SPECASM_DOC_CPI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cpi,
		"XX X X1 ",
	},
	{
		"cpir",
		SPECASM_DOC_CPIR_FORMS,
		SPECASM_DOC_CPIR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cpir,
		"XX X X1 ",
	},
	{
		"cpl",
		SPECASM_DOC_CPL_FORMS,
		SPECASM_DOC_CPL_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_cpl,
		"   1  1 ",
	},
	{
		"daa",
		SPECASM_DOC_DAA_FORMS,
		SPECASM_DOC_DAA_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_daa,
		"XX X X X",
	},
	{
		"db",
		SPECASM_DOC_DB_FORMS,
		SPECASM_DOC_DB_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_db,
		NULL,
	},
	{
		"dec",
		SPECASM_DOC_DEC1_FORMS,
		SPECASM_DOC_DEC1_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_dec,
		"XX X X1 ",
	},
	{
		"dec",
		SPECASM_DOC_DEC2_FORMS,
		SPECASM_DOC_DEC2_NUM_FORMS,
		3,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_dec,
		NULL,
	},
	{
		"di",
		SPECASM_DOC_DI_FORMS,
		SPECASM_DOC_DI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_di,
		NULL,
	},
	{
		"djnz",
		SPECASM_DOC_DJNZ_FORMS,
		SPECASM_DOC_DJNZ_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_djnz,
		NULL,
	},
	{
		"ds",
		SPECASM_DOC_DS_FORMS,
		SPECASM_DOC_DS_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ds,
		NULL,
	},
	{
		"dw",
		SPECASM_DOC_DW_FORMS,
		SPECASM_DOC_DW_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_dw,
		NULL,
	},
	{
		"ei",
		SPECASM_DOC_EI_FORMS,
		SPECASM_DOC_EI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ei,
		NULL,
	},
	{
		"ex",
		SPECASM_DOC_EX_FORMS,
		SPECASM_DOC_EX_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ex,
		"XXXXXXXX",
	},
	{
		"exx",
		SPECASM_DOC_EXX_FORMS,
		SPECASM_DOC_EXX_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_exx,
		NULL,
	},
	{
		"halt",
		SPECASM_DOC_HALT_FORMS,
		SPECASM_DOC_HALT_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_halt,
		NULL,
	},
	{
		"im",
		SPECASM_DOC_IM_FORMS,
		SPECASM_DOC_IM_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_im,
		NULL,
	},
	{
		"in",
		SPECASM_DOC_IN1_FORMS,
		SPECASM_DOC_IN1_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_in,
		"XX X X0 ",
	},
	{
		"in",
		SPECASM_DOC_IN2_FORMS,
		SPECASM_DOC_IN2_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_in2,
		NULL,
	},
	{
		"inc",
		SPECASM_DOC_INC1_FORMS,
		SPECASM_DOC_INC1_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
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
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ind,
		"?X ? ?1 ",
	},
	{
		"indr",
		SPECASM_DOC_INDR_FORMS,
		SPECASM_DOC_INDR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_indr,
		"?1 ? ?1 "
	},
	{
		"ini",
		SPECASM_DOC_INI_FORMS,
		SPECASM_DOC_INI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ini,
		"?X ? ?1 ",
	},
	{
		"inir",
		SPECASM_DOC_INIR_FORMS,
		SPECASM_DOC_INIR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_inir,
		"?1 ? ?1 "
	},
	{
		"jp",
		SPECASM_DOC_JP_FORMS,
		SPECASM_DOC_JP_NUM_FORMS,
		0,
		0,
		8,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_jp,
		NULL,
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"jp",
		SPECASM_DOC_JP_C_FORMS,
		SPECASM_DOC_JP_C_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_jp_c,
		"?? ? ???"
	},
#endif
	{
		"jr",
		SPECASM_DOC_JR_FORMS,
		SPECASM_DOC_JR_NUM_FORMS,
		0,
		0,
		8,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_jr,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD1_FORMS,
		SPECASM_DOC_LD1_NUM_FORMS,
		3,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld1,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD2_FORMS,
		SPECASM_DOC_LD2_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld2,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD3_FORMS,
		SPECASM_DOC_LD3_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld3,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD4_FORMS,
		SPECASM_DOC_LD4_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld4,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD5_FORMS,
		SPECASM_DOC_LD5_NUM_FORMS,
		3,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld5,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD6_FORMS,
		SPECASM_DOC_LD6_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld6,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD7_FORMS,
		SPECASM_DOC_LD7_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld7,
		NULL,
	},
	{
		"ld",
		SPECASM_DOC_LD8_FORMS,
		SPECASM_DOC_LD8_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ld8,
		NULL,
	},
	{
		"ldd",
		SPECASM_DOC_LDD_FORMS,
		SPECASM_DOC_LDD_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ldd,
		"    0 X0 ",
	},
	{
		"lddr",
		SPECASM_DOC_LDDR_FORMS,
		SPECASM_DOC_LDDR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_lddr,
		"   0 00 ",
	},

#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"lddrx",
		SPECASM_DOC_LDDRX_FORMS,
		SPECASM_DOC_LDDRX_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_lddrx,
		NULL,
	},
	{
		"lddx",
		SPECASM_DOC_LDDX_FORMS,
		SPECASM_DOC_LDDX_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_lddx,
		NULL,
	},
#endif
	{
		"ldi",
		SPECASM_DOC_LDI_FORMS,
		SPECASM_DOC_LDI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ldi,
		"    0 X0 ",
	},
	{
		"ldir",
		SPECASM_DOC_LDIR_FORMS,
		SPECASM_DOC_LDIR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ldir,
		"   0 00 ",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"ldirx",
		SPECASM_DOC_LDIRX_FORMS,
		SPECASM_DOC_LDIRX_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_ldirx,
		NULL,
	},
	{
		"ldix",
		SPECASM_DOC_LDIX_FORMS,
		SPECASM_DOC_LDIX_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_ldix,
		NULL,
	},
	{
		"ldpirx",
		SPECASM_DOC_LDPIRX_FORMS,
		SPECASM_DOC_LDPIRX_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_ldpirx,
		NULL,
	},
	{
		"ldws",
		SPECASM_DOC_LDWS_FORMS,
		SPECASM_DOC_LDWS_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_ldws,
		"XX X X0 ",
	},
#endif
	{
		"map",
		SPECASM_DOC_MAP_FORMS,
		SPECASM_DOC_MAP_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_map,
		NULL,
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"mirror",
		SPECASM_DOC_MIRROR_FORMS,
		SPECASM_DOC_MIRROR_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_mirror,
		NULL,
	},
	{
		"mul",
		SPECASM_DOC_MUL_FORMS,
		SPECASM_DOC_MUL_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_mul,
		NULL,
	},
#endif
	{
		"neg",
		SPECASM_DOC_NEG_FORMS,
		SPECASM_DOC_NEG_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_neg,
		"XX X X1X",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"nextreg",
		SPECASM_DOC_NEXTREG_FORMS,
		SPECASM_DOC_NEXTREG_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_nextreg,
		NULL,
	},
#endif
	{
		"nop",
		SPECASM_DOC_NOP_FORMS,
		SPECASM_DOC_NOP_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_nop,
		NULL,
	},
	{
		"or",
		SPECASM_DOC_OR_FORMS,
		SPECASM_DOC_OR_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_or,
		"XX 0 X00",
	},
	{
		"org",
		SPECASM_DOC_ORG_FORMS,
		SPECASM_DOC_ORG_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_org,
		NULL,
	},
	{
		"out",
		SPECASM_DOC_OUT1_FORMS,
		SPECASM_DOC_OUT1_NUM_FORMS,
		6,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_out1,
		NULL,
	},
	{
		"out",
		SPECASM_DOC_OUT2_FORMS,
		SPECASM_DOC_OUT2_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_out2,
		NULL,
	},
	{
		"outd",
		SPECASM_DOC_OUTD_FORMS,
		SPECASM_DOC_OUTD_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_outd,
		"?X ? ?1 ",
	},
	{
		"outdr",
		SPECASM_DOC_OUTDR_FORMS,
		SPECASM_DOC_OUTDR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_outdr,
		"?1 ? ?1 ",
	},
	{
		"outi",
		SPECASM_DOC_OUTI_FORMS,
		SPECASM_DOC_OUTI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_outi,
		"?X ? ?1 ",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"outinb",
		SPECASM_DOC_OUTINB_FORMS,
		SPECASM_DOC_OUTINB_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_outinb,
		"?? ? ???",
	},
#endif
	{
		"outir",
		SPECASM_DOC_OUTIR_FORMS,
		SPECASM_DOC_OUTIR_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_outir,
		"?1 ? ?1 ",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"pixelad",
		SPECASM_DOC_PIXELAD_FORMS,
		SPECASM_DOC_PIXELAD_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_pixelad,
		NULL,
	},
	{
		"pixeldn",
		SPECASM_DOC_PIXELDN_FORMS,
		SPECASM_DOC_PIXELDN_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_pixeldn,
		NULL,
	},
#endif
	{
		"pop",
		SPECASM_DOC_POP_FORMS,
		SPECASM_DOC_POP_NUM_FORMS,
		7,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_pop,
		NULL,
	},
	{
		"push",
		SPECASM_DOC_PUSH_FORMS,
		SPECASM_DOC_PUSH_NUM_FORMS,
		7,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_push,
		NULL,
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"push",
		SPECASM_DOC_PUSH_IMM_FORMS,
		SPECASM_DOC_PUSH_IMM_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_push_imm,
		NULL,
	},
#endif
	{
		"res",
		SPECASM_DOC_RES_FORMS,
		SPECASM_DOC_RES_NUM_FORMS,
		2,
		8,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_res,
		NULL,
	},
	{
		"ret",
		SPECASM_DOC_RET_FORMS,
		SPECASM_DOC_RET_NUM_FORMS,
		0,
		0,
		8,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_ret,
		NULL,
	},
	{
		"reti",
		SPECASM_DOC_RETI_FORMS,
		SPECASM_DOC_RETI_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_reti,
		NULL,
	},
	{
		"retn",
		SPECASM_DOC_RETN_FORMS,
		SPECASM_DOC_RETN_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_retn,
		NULL,
	},
	{
		"rl",
		SPECASM_DOC_RL_FORMS,
		SPECASM_DOC_RL_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rl,
		"XX 0 X0X",
	},
	{
		"rla",
		SPECASM_DOC_RLA_FORMS,
		SPECASM_DOC_RLA_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rla,
		"   0  0X",
	},
	{
		"rlc",
		SPECASM_DOC_RLC_FORMS,
		SPECASM_DOC_RLC_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rlc,
		"XX 0 X0X",
	},
	{
		"rlca",
		SPECASM_DOC_RLCA_FORMS,
		SPECASM_DOC_RLCA_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rlca,
		"   0  0X",
	},
	{
		"rld",
		SPECASM_DOC_RLD_FORMS,
		SPECASM_DOC_RLD_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rld,
		"XX 0 X0 ",
	},
	{
		"rr",
		SPECASM_DOC_RR_FORMS,
		SPECASM_DOC_RR_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rr,
		"XX 0 X0X",
	},
	{
		"rra",
		SPECASM_DOC_RRA_FORMS,
		SPECASM_DOC_RRA_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rra,
		"   0  0X",
	},
	{
		"rrc",
		SPECASM_DOC_RRC_FORMS,
		SPECASM_DOC_RRC_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rrc,
		"XX 0 X0X",
	},
	{
		"rrca",
		SPECASM_DOC_RRCA_FORMS,
		SPECASM_DOC_RRCA_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rrca,
		"   0  0X",
	},
	{
		"rrd",
		SPECASM_DOC_RRD_FORMS,
		SPECASM_DOC_RRD_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rrd,
		"XX 0 X0 ",
	},
	{
		"rst",
		SPECASM_DOC_RST_FORMS,
		SPECASM_DOC_RST_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_rst,
		NULL,
	},
	{
		"sbc",
		SPECASM_DOC_SBC_FORMS,
		SPECASM_DOC_SBC_NUM_FORMS,
		1,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_sbc,
		"XX X X1X",
	},
	{
		"scf",
		SPECASM_DOC_SCF_FORMS,
		SPECASM_DOC_SCF_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_scf,
		"   0  01"
	},
	{
		"set",
		SPECASM_DOC_SET_FORMS,
		SPECASM_DOC_SET_NUM_FORMS,
		2,
		8,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_set,
		NULL,
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"setae",
		SPECASM_DOC_SETAE_FORMS,
		SPECASM_DOC_SETAE_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_setae,
		NULL,
	},
#endif
	{
		"sla",
		SPECASM_DOC_SLA_FORMS,
		SPECASM_DOC_SLA_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_sla,
		"XX 0 X0X",
	},
	{
		"sra",
		SPECASM_DOC_SRA_FORMS,
		SPECASM_DOC_SRA_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_sra,
		"XX 0 X0X",
	},
	{
		"srl",
		SPECASM_DOC_SRL_FORMS,
		SPECASM_DOC_SRL_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_srl,
		"XX 0 X0X",
	},
	{
		"sub",
		SPECASM_DOC_SUB_FORMS,
		SPECASM_DOC_SUB_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_sub,
		"XX X X1X",
	},
#ifdef SPECASM_TARGET_NEXT_OPCODES
	{
		"swapnib",
		SPECASM_DOC_SWAPNIB_FORMS,
		SPECASM_DOC_SWAPNIB_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_swapnib,
		NULL,
	},
	{
		"test",
		SPECASM_DOC_TEST_FORMS,
		SPECASM_DOC_TEST_NUM_FORMS,
		0,
		0,
		0,
		1,
		specasm_doc_test,
		"XX X X?X",
	},
#endif
	{
		"xor",
		SPECASM_DOC_XOR_FORMS,
		SPECASM_DOC_XOR_NUM_FORMS,
		2,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_xor,
		"XX 0 X00",
	},
	{
		"zx81",
		SPECASM_DOC_MAP_FORMS,
		SPECASM_DOC_MAP_NUM_FORMS,
		0,
		0,
		0,
#ifdef SPECASM_TARGET_NEXT_OPCODES
		0,
#endif
		specasm_doc_zx81,
		NULL,
	},
};

/* clang-format on */

const uint8_t max_docs = sizeof(docs) / sizeof(specasm_ins_doc_t);

static uint8_t prv_pretty_print(uint8_t y, const char *text)
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
	const char *reg_name;
	char *s2;
	char *rr_start = NULL;
	const uint8_t *enc;
	char *s = scratch;

	if (!doc->reg_encoding)
		return y;

	enc = (const uint8_t *)&reg_encodings[doc->reg_encoding - 1];

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
			s += 3;
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
			s += 3;
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

static uint8_t prv_print_bits_encoding(const specasm_ins_doc_t *doc, uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	char *s;

	if (!doc->bits)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[15] = 'n';
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
		x = (i * 4) + 1;
		if (num < 16)
			x++;
		specasm_util_print(scratch, x, y, PAPER_BLACK | INK_WHITE);
	}

	return y + 2;
}

static uint8_t prv_print_cc_encoding(const specasm_ins_doc_t *doc, uint8_t y)
{
	uint8_t i;
	uint8_t x;
	uint8_t num;
	uint8_t limit;
	char *s;

	if (!doc->all_cc)
		return y;

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[14] = 'c';
	scratch[15] = 'c';
	specasm_util_print(scratch, 0, y, PAPER_WHITE | INK_BLACK);
	y++;

	if (strcmp(doc->name, "jr")) {
		limit = 8;
		specasm_util_print(" nz   z  nc   c  po  pe   p   m ", 0, y,
				   PAPER_BLUE | INK_WHITE);
	} else {
		limit = 4;
		specasm_util_print(" nz   z  nc   c ", 0, y,
				   PAPER_BLUE | INK_WHITE);
	}
	y++;

	for (i = 0; i < limit; i++) {
		num = doc->all_cc * i;
		itoa(num, scratch, 16);
		x = (i * 4) + 1;
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
	const specasm_ins_doc_t *doc = &docs[ins_id];
	const specasm_ins_form_t *form;
	const char *ins_name = doc->name;
	uint8_t ins_name_len = strlen(ins_name);

	specasm_cls(PAPER_BLACK | INK_WHITE);
	x = (SPECASM_LINE_MAX_LEN / 2) - (strlen(ins_name) >> 1);

	memset(scratch, ' ', SPECASM_LINE_MAX_LEN);
	scratch[SPECASM_LINE_MAX_LEN] = 0;
	memcpy(&scratch[x], ins_name, ins_name_len);

	specasm_util_print(scratch, 0, 0, PAPER_BLUE | INK_WHITE | 64);
#ifdef SPECASM_TARGET_NEXT_OPCODES
	if (doc->zxn)
		specasm_util_print("zxn", 0, 0, PAPER_BLUE | INK_RED | 64);
#endif
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
#ifdef SPECASM_NEXT_BANKED
	specasm_descr_bank(ins_name);
#endif
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
	r = max_docs - 1;

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
			for (; m > 0 && !strcmp(docs[m - 1].name, ins_name);
			     m--)
				;
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
	char jmp[2];
	uint8_t redraw = 1;

	jmp[1] = 0;

	id = prv_find_mnemomic(ins_name);
	do {
		if (redraw)
			prv_draw_help(id);
		do {
			specasm_sleep_ms(25);
		} while (!(k = in_inkey()));

		redraw = 0;
		if (k == SPECASM_KEY_LEFT) {
			if (id > 0) {
				id--;
				redraw = 1;
			}
		} else if (k == SPECASM_KEY_RIGHT) {
			if (id < max_docs - 1) {
				id++;
				redraw = 1;
			}
		} else if (k >= 'a' && k <= 'z') {
			jmp[0] = k;
			id = prv_find_mnemomic(jmp);
			if (id < max_docs - 1)
				id++;
			redraw = 1;
		} else {
			break;
		}
	} while (1);

	specasm_cls(PAPER_BLACK | INK_WHITE);
}
