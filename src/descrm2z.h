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

#ifndef DESCRM2Z_H
#define DESCRM2Z_H

extern const char specasm_doc_map[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_mirror[];
extern const char specasm_doc_mul[];
#endif

extern const char specasm_doc_neg[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_nextreg[];
#endif

extern const char specasm_doc_nop[];
extern const char specasm_doc_or[];
extern const char specasm_doc_org[];
extern const char specasm_doc_out1[];
extern const char specasm_doc_out2[];
extern const char specasm_doc_outi[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_outinb[];
#endif

extern const char specasm_doc_outir[];
extern const char specasm_doc_outd[];
extern const char specasm_doc_outdr[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_pixelad[];
extern const char specasm_doc_pixeldn[];
#endif

extern const char specasm_doc_pop[];
extern const char specasm_doc_push[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_push_imm[];
#endif

extern const char specasm_doc_res[];
extern const char specasm_doc_ret[];
extern const char specasm_doc_reti[];
extern const char specasm_doc_retn[];
extern const char specasm_doc_rl[];
extern const char specasm_doc_rla[];
extern const char specasm_doc_rlc[];
extern const char specasm_doc_rlca[];
extern const char specasm_doc_rld[];
extern const char specasm_doc_rr[];
extern const char specasm_doc_rra[];
extern const char specasm_doc_rrc[];
extern const char specasm_doc_rrca[];
extern const char specasm_doc_rrd[];
extern const char specasm_doc_rst[];
extern const char specasm_doc_sbc[];
extern const char specasm_doc_scf[];
extern const char specasm_doc_set[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_setae[];
#endif

extern const char specasm_doc_sla[];
extern const char specasm_doc_sra[];
extern const char specasm_doc_srl[];
extern const char specasm_doc_sub[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_swapnib[];
extern const char specasm_doc_test[];
#endif

extern const char specasm_doc_xor[];
extern const char specasm_doc_zx81[];

#endif