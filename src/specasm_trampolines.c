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
#include <arch/zxn.h>

#include "line.h"

#define SPECASM_NEXT_PARSE_BANK ((43<<1)+1)
#define SPECASM_NEXT_DUMP_BANK ((44<<1)+1)
#define SPECASM_NEXT_PARSE_LD_BANK ((45<<1)+1)

extern unsigned char _z_page_table[];

typedef struct specasm_opcode_t_ specasm_opcode_t;

uint8_t specasm_parse_exp_banked_e(const char *str, uint8_t *label1,
				   uint8_t *label1_type);
uint8_t specasm_parse_mnemomic_banked_e(const char *str, uint8_t i,
					specasm_line_t *line);
void specasm_init_dump_table_banked(void);
uint8_t specasm_dump_opcode_banked_e(const specasm_line_t *line, char *buf);
uint8_t specasm_parse_ld_banked_e(const char *args, specasm_line_t *line,
				  const specasm_opcode_t *op_entry);

uint8_t specasm_parse_exp_e(const char *str, uint8_t *label1,
			    uint8_t *label1_type)
{
	uint8_t e;
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	e = specasm_parse_exp_banked_e(str, label1, label1_type);

	return e;
}

uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line)
{
	uint8_t e;

	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	e = specasm_parse_mnemomic_banked_e(str, i, line);

	return e;
}

void specasm_init_dump_table(void)
{
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_DUMP_BANK]);
	specasm_init_dump_table_banked();
}

uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf)
{
	uint8_t e;

	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_DUMP_BANK]);
	e = specasm_dump_opcode_banked_e(line, buf);

	return e;
}

uint8_t specasm_parse_ld_e(const char *args, specasm_line_t *line,
			   const specasm_opcode_t *op_entry)
{
	uint8_t e;

	/*
	 * This one is a little tricky.  It's actually called not
	 * from the main program but from another bank.  So we need to
	 * switch back to the parse bank when we've finished.
	 */

	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_LD_BANK]);
	e = specasm_parse_ld_banked_e(args, line, op_entry);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK]);

	return e;
}

