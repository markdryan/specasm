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

#ifndef LINE_PARSE_COMMON_H
#define LINE_PARSE_COMMON_H

#include <stdint.h>

char *specasm_get_uword_imm_e(const char *str, uint16_t *val, uint8_t *flags);
char *specasm_get_long_imm_e(const char *str, long *val, uint8_t *flags);
uint8_t specasm_parse_reg_e(const char *str, uint8_t *r, uint8_t *off,
			    uint8_t *flags);
const char *specasm_parse_jump_label_e(const char *args,
				       specasm_line_t *line, uint8_t *label);
const char *
specasm_parse_label_or_exp_e(const char *args, specasm_line_t *line,
			     uint8_t *label);
const char *specasm_get_char_imm_e(const char *str, uint8_t *val,
				   uint8_t *flags);
const char *specasm_get_word_imm_e(const char *str, uint16_t *val,
				   uint8_t *flags);
const char *specasm_parse_word_imm_or_exp_e(const char *args,
					    specasm_line_t *line,
					    uint16_t *val, uint8_t *flags);
const char *specasm_get_exp_e(specasm_line_t *line, const char *args,
			      uint8_t *val);
const char *specasm_get_byte_imm_e(const char *str, uint8_t *val,
				   uint8_t *flags);
const char *specasm_get_byte_imm_ind_e(specasm_line_t *line,
				       const char *args, uint8_t *val);
const char *specasm_parse_byte_imm_or_exp_e(const char *args,
					    specasm_line_t *line,
					    uint8_t *val, uint8_t *label);
const char *specasm_parse_reg_comma_e(const char *args, uint8_t *r,
				      uint8_t *off, uint8_t *flags);
#endif
