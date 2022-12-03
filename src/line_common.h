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

#ifndef LINE_COMMON_H_
#define LINE_COMMON_H_

#include <stdint.h>

#include "line.h"

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

extern char byte_regs[8];

struct specasm_mnemomic_t_ {
	const char *mnemomic;
	uint8_t line_type;
};

typedef struct specasm_mnemomic_t_ specasm_mnemomic_t;

extern const specasm_mnemomic_t mnemomics_table[];
extern const uint8_t mnemomics_table_size;

#endif
