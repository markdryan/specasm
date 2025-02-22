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

#ifndef DESCRA2L_H
#define DESCRA2L_H

extern const char specasm_doc_adc[];
extern const char specasm_doc_add[];
extern const char specasm_doc_add_16[];
#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_add_rra[];
extern const char specasm_doc_add_16imm[];
#endif
extern const char specasm_doc_align[];
extern const char specasm_doc_and[];
extern const char specasm_doc_bit[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_brlc[];
extern const char specasm_doc_bsla[];
extern const char specasm_doc_bsra[];
extern const char specasm_doc_bsrf[];
extern const char specasm_doc_bsrl[];
#endif

extern const char specasm_doc_call[];
extern const char specasm_doc_ccf[];
extern const char specasm_doc_cp[];
extern const char specasm_doc_cpd[];
extern const char specasm_doc_cpdr[];
extern const char specasm_doc_cpi[];
extern const char specasm_doc_cpir[];
extern const char specasm_doc_cpl[];
extern const char specasm_doc_daa[];
extern const char specasm_doc_db[];
extern const char specasm_doc_dec[];
extern const char specasm_doc_di[];
extern const char specasm_doc_djnz[];
extern const char specasm_doc_ds[];
extern const char specasm_doc_dw[];
extern const char specasm_doc_ei[];
extern const char specasm_doc_ex[];
extern const char specasm_doc_exx[];
extern const char specasm_doc_halt[];
extern const char specasm_doc_im[];
extern const char specasm_doc_in[];
extern const char specasm_doc_in2[];
extern const char specasm_doc_inc[];
extern const char specasm_doc_ind[];
extern const char specasm_doc_indr[];
extern const char specasm_doc_ini[];
extern const char specasm_doc_inir[];
extern const char specasm_doc_jp[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_jp_c[];
#endif

extern const char specasm_doc_jr[];
extern const char specasm_doc_ld1[];
extern const char specasm_doc_ld2[];
extern const char specasm_doc_ld3[];
extern const char specasm_doc_ld4[];
extern const char specasm_doc_ld5[];
extern const char specasm_doc_ld6[];
extern const char specasm_doc_ld7[];
extern const char specasm_doc_ld8[];
extern const char specasm_doc_ldd[];
extern const char specasm_doc_lddr[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_lddrx[];
extern const char specasm_doc_lddx[];
#endif

extern const char specasm_doc_ldi[];
extern const char specasm_doc_ldir[];

#ifdef SPECASM_TARGET_NEXT_OPCODES
extern const char specasm_doc_ldirx[];
extern const char specasm_doc_ldix[];
extern const char specasm_doc_ldpirx[];
extern const char specasm_doc_ldws[];
#endif

#endif