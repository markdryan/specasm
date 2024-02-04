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

#include <arch/zxn.h>
#include <stdint.h>

#include "salink.h"

#define SALINK_NEXT_EXPRESSION_BANK ((43 << 1) + 1)
#define SALINK_NEXT_MAP_BANK ((44 << 1) + 1)
#define SALINK_NEXT_LINK_OBJ_BANK ((45 << 1) + 1)

extern unsigned char _z_page_table[];

void salink_apply_expressions_banked_e(specasm_line_t *line, salink_obj_t *obj,
				       unsigned int line_no);
void salink_equ_eval_global_banked_e(salink_obj_t *obj, salink_global_t *global,
				     salink_label_t *label, uint8_t depth);
void specasm_write_map_banked_e(void);
int salink_link_banked_e(void);

int salink_link_e(void)
{
	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_LINK_OBJ_BANK]);
	return salink_link_banked_e();
}

void specasm_write_map_e(void)
{
	/*
	 * Only called from the link_obj page, so we map it back
	 * in once our call has finished.
	 */

	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_MAP_BANK]);
	specasm_write_map_banked_e();
	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_LINK_OBJ_BANK]);
}

void salink_apply_expressions_e(specasm_line_t *line, salink_obj_t *obj,
				unsigned int line_no)
{
	/*
	 * Only called from the link_obj page, so we map it back
	 * in once our call has finished.
	 */

	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_EXPRESSION_BANK]);
	salink_apply_expressions_banked_e(line, obj, line_no);
	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_LINK_OBJ_BANK]);
}

void salink_equ_eval_global_e(salink_obj_t *obj, salink_global_t *global,
			      salink_label_t *label, uint8_t depth)
{
	/*
	 * Only called from the link_obj page, so we map it back
	 * in once our call has finished.
	 */

	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_EXPRESSION_BANK]);
	salink_equ_eval_global_banked_e(obj, global, label, depth);
	ZXN_WRITE_MMU7(_z_page_table[SALINK_NEXT_LINK_OBJ_BANK]);
}
