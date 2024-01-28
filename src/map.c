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
#include <string.h>

#include "map.h"
#include "peer.h"
#include "salink.h"
#include "state_base.h"

static void prv_write_buffered_e(specasm_handle_t f, const char *str)
{
	int len = strlen(str);

	if (len + buf_count > MAX_BUFFER_SIZE) {
		specasm_file_write_e(f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			return;
		buf_count = 0;
	}
	memcpy(&buf.file_buf[buf_count], str, len);
	buf_count += len;
}

static void prv_dump_globals_e(specasm_handle_t f, salink_global_t *glob,
			       salink_obj_t *obj)
{
	char ibuf[16];
	uint8_t type = labels[glob->label_index].type;

	if (type > SALINK_LABEL_TYPE_LNG)
		return;

	ibuf[0] = '$';
	itoa(labels[glob->label_index].data.off, &ibuf[1], 16);
	prv_write_buffered_e(f, ibuf);
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_write_buffered_e(f, " - ");
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_write_buffered_e(f, obj->fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_write_buffered_e(f, ":");
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_write_buffered_e(f, glob->name);
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_write_buffered_e(f, "\n");
}

#ifdef SPECASM_NEXT_BANKED
void specasm_write_map_banked_e(void)
#else
void specasm_write_map_e(void)
#endif
{
	unsigned int of;
	unsigned int i;
	specasm_handle_t f;
	salink_global_t *glob;
	salink_obj_t *obj;
	salink_label_t *label;
	const char *str;
	char ibuf[16];

	ibuf[0] = '$';

	(void)specasm_text_print("     ", SALINK_VAL_COL + 1,
				 SALINK_FIELD_FILES_ROW, SPECASM_CODE_COLOUR);

	f = specasm_file_wopen_e(map_name);
	if (err_type != SPECASM_ERROR_OK)
		return;

	buf_count = 0;
	prv_write_buffered_e(f, "Globals\n-------\n");
	for (i = 0; i < global_count; i++) {
		glob = &globals[i];
		if (glob->obj_index != 0)
			continue;
		obj = &obj_files[glob->obj_index];
		prv_dump_globals_e(f, glob, obj);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}
	for (i = 0; i < global_count; i++) {
		glob = &globals[i];
		if (glob->obj_index == 0)
			continue;
		obj = &obj_files[glob->obj_index];
		prv_dump_globals_e(f, glob, obj);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	for (of = 0; of < obj_file_count; of++) {
		itoa(of + 1, &ibuf[1], 10);
		(void)specasm_text_print(&ibuf[1], SALINK_VAL_COL + 1,
					 SALINK_FIELD_FILES_ROW,
					 SPECASM_CODE_COLOUR);

		obj = &obj_files[of];
		specasm_load_e(obj->fname);
		if (err_type != SPECASM_ERROR_OK)
			return;

		prv_write_buffered_e(f, "\n");
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
		prv_write_buffered_e(f, obj->fname);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
		prv_write_buffered_e(f, "\n-------\n");
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
		for (i = obj->label_start; i < obj->label_end; i++) {
			label = &labels[i];
			if (label->type > SALINK_LABEL_TYPE_LNG)
				continue;
			str = salink_get_label_str_e(label->id, label->type);
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
			itoa(label->data.off, &ibuf[1], 16);
			prv_write_buffered_e(f, ibuf);
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
			prv_write_buffered_e(f, " - ");
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
			prv_write_buffered_e(f, str);
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
			prv_write_buffered_e(f, "\n");
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
		}
	}

	if (buf_count > 0) {
		specasm_file_write_e(f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	specasm_file_close_e(f);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	return;

on_error:

	specasm_file_close_e(f);
	specasm_remove_file(image_name);

	return;
}
