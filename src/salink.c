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
char error_buf[(SPECASM_LINE_MAX_LEN * 4) + 1];
salink_buf_t buf;
unsigned int buf_count;
unsigned int bin_size;
char image_name[MAX_FNAME + 1];
char map_name[MAX_FNAME + 1];

salink_label_t labels[MAX_LABELS];
salink_obj_t obj_files[MAX_FILES];
uint8_t obj_file_count;
uint8_t obj_files_order[MAX_FILES];
salink_global_t globals[MAX_GLOBALS];
unsigned int global_count;
uint8_t queued_files;
uint8_t link_mode;
uint8_t got_test;
uint8_t got_zx81;

const char *empty_str = "";
const char *specasm_str = "/specasm/";

#ifdef SPECASM_TARGET_NEXT
void specasm_peer_next_copy_chars(void);
#endif

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
