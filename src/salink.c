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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SPECASM_TARGET_NEXT
#include <arch/zxn/esxdos.h>
#endif

#include "expression.h"
#include "link_obj.h"
#include "map.h"
#include "peer.h"
#include "salink.h"
#include "state_base.h"

char scratch[SPECASM_MAX_SCRATCH];
char error_buf[(SPECASM_LINE_MAX_LEN * 3) + 1];
salink_buf_t buf;
unsigned int buf_count;
unsigned int bin_size;
char image_name[MAX_FNAME + 1];
char map_name[MAX_FNAME + 1];

salink_label_t labels[MAX_LABELS];
salink_obj_t obj_files[MAX_FILES];
unsigned int obj_file_count;
uint8_t obj_files_order[MAX_FILES];
salink_global_t globals[MAX_GLOBALS];
unsigned int global_count;

#ifdef SPECASM_TARGET_NEXT
void specasm_peer_next_copy_chars(void);
#endif

static void prv_unknown_error_label_e(salink_obj_t *obj, const char *str)
{
	snprintf(error_buf, sizeof(error_buf), "%s:Unknown label in equ:%s",
		 obj->fname, str);
	err_type = SALINK_ERROR_UNRESOLVED_LABEL;
}

/*
 * Returns the string from the state of the currently loaded object
 * file identified by the given id and type.
 *
 * If label_type is odd id is expected to be a long string, otherwise
 * it is expected to be a short string.
 */
const char *salink_get_label_str_e(uint8_t id, uint8_t label_type)
{
	if (label_type & 1)
		return specasm_state_get_long_e(id);
	else
		return specasm_state_get_short_e(id);
}

/*
 * Find a label id given a string.  We need this when evaluating expressions.
 * Labels within expressions are encoded as strings rather than as ids.
 */

unsigned int salink_find_local_label_e(const char *str, int len,
				       salink_obj_t *obj)
{
	unsigned int i;
	salink_label_t *label;
	const char *lab_str;
	uint8_t lng = len > SPECASM_MAX_SHORT_LEN ? 1 : 0;
	uint8_t lab_lng;

	for (i = obj->label_start; i < obj->label_end; i++) {
		label = &labels[i];
		if ((label->type == SALINK_LABEL_TYPE_ALIGN) ||
		    (label->type == SALINK_LABEL_TYPE_EQU_GLOBAL) ||
		    (label->type == SALINK_LABEL_TYPE_EQU_EVAL_GLOBAL))
			continue;
		lab_lng = label->type & 1;
		if (lng != lab_lng)
			continue;

		lab_str = salink_get_label_str_e(label->id, lng);
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		if (!strcmp(str, lab_str))
			return i;
	}

	prv_unknown_error_label_e(obj, str);

	return 0;
}

/*
 * Returns the index of the global that matches str
 */

unsigned int salink_find_global_label_e(const char *str, salink_obj_t *obj)
{
	unsigned int i;
	salink_global_t *global;

	for (i = 0; i < global_count; i++) {
		global = &globals[i];
		if (!strcmp(str, global->name))
			return i;
	}

	prv_unknown_error_label_e(obj, str);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

#ifdef SPECASM_TARGET_NEXT
	uint8_t turbo;
	struct esx_mode mode;

	memset(&mode, 0, sizeof(mode));
	(void)esx_ide_mode_set(&mode);

	/*
	 * Link at top speed on a spectrum Next.
	 */

	turbo = ZXN_READ_REG(REG_TURBO_MODE);
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo | 3);
	zx_cls(PAPER_WHITE | INK_WHITE);
	specasm_peer_next_copy_chars();
	zx_cls(PAPER_WHITE | INK_BLACK);
#endif

	ret = salink_link_e();

#ifdef SPECASM_TARGET_NEXT
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo);
#endif
	return ret;
}
