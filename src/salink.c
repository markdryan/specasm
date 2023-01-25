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

#include "expression.h"
#include "peer.h"
#include "peer_file.h"
#include "salink.h"
#include "state_base.h"

#define MAX_PENDING_X_FILES (MAX_BUFFER_SIZE / (MAX_FNAME + 1))

#define SALINK_FIELD_COL 1
#define SALINK_VAL_COL 15
#define SALINK_FIELD_NAME_ROW 2
#define SALINK_FIELD_STARTADDR_ROW 4
#define SALINK_FIELD_FILES_ROW 6
#define SALINK_FIELD_GLOBALS_ROW 8
#define SALINK_FIELD_SIZE_ROW 10
#define SALINK_FIELD_MAP_ROW 12
#define SALINK_FIELD_MAX_ROW SALINK_FIELD_MAP_ROW
#define SALINK_STATUS_ROW SALINK_FIELD_MAX_ROW + 2

char scratch[SPECASM_MAX_SCRATCH];
char error_buf[(SPECASM_LINE_MAX_LEN * 3) + 1];
static union {
	char file_buf[MAX_BUFFER_SIZE];
	char fname[MAX_PENDING_X_FILES][MAX_FNAME + 1];
} buf;
static uint8_t queued_files;
static unsigned int buf_count;
static unsigned int bin_size;
static char image_name[MAX_FNAME + 1];
static char map_name[MAX_FNAME + 1];
static uint16_t start_address = 0x8000;
static uint8_t got_org;
static uint8_t map_file;

salink_label_t labels[MAX_LABELS];
static size_t label_count;
static salink_obj_t obj_files[MAX_FILES];
static unsigned int obj_file_count;
salink_global_t globals[MAX_GLOBALS];
static unsigned int global_count;

size_t main_index = SIZE_MAX;

static specasm_dirent_t dirent;

static int prv_check_file(const char *fname)
{
	char *period;

	period = strchr(fname, '.');

	if (!period)
		return 0;

	return ((period[1] == 'x' || period[1] == 'X') && period[2] == 0);
}

static void prv_init_out_fnames(salink_obj_t *obj)
{
	unsigned int i;

	for (i = 0; i < MAX_FNAME; i++) {
		if (obj->fname[i] == '.')
			break;
		image_name[i] = obj->fname[i];
	}
	image_name[i] = 0;

	(void)specasm_text_print(image_name, SALINK_VAL_COL + 1,
				 SALINK_FIELD_NAME_ROW, SPECASM_CODE_COLOUR);

	strcpy(map_name, image_name);
	if (i + 4 > MAX_FNAME)
		i = MAX_FNAME - 4;
	map_name[i] = '.';
	map_name[i + 1] = 'm';
	map_name[i + 2] = 'a';
	map_name[i + 3] = 'p';
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

static uint8_t prv_add_label_e(salink_obj_t *obj, uint16_t size, uint8_t type,
			       uint8_t id, uint16_t line_no)
{
	char ibuf[16];
	unsigned int i;
	const char *str;
	const char *str1;
	salink_label_t *label;
	salink_global_t *global;
	uint8_t retval = 0;

	if (label_count == MAX_LABELS) {
		snprintf(error_buf, sizeof(error_buf),
			 "Max label limit %d reached", MAX_LABELS);
		err_type = SALINK_ERROR_TOO_MANY_LABELS;
		return 0;
	}
	label = &labels[label_count];
	label->id = id;
	label->type = type;
	label->data.off = size;
	str = salink_get_label_str_e(id, type);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	if (str[0] >= 'A' && str[0] <= 'Z') {
		if (global_count == MAX_GLOBALS) {
			snprintf(error_buf, sizeof(error_buf),
				 "Global limit %d reached", MAX_GLOBALS);
			err_type = SALINK_ERROR_TOO_MANY_GLOBALS;
			return 0;
		}
		for (i = 0; i < global_count; i++) {
			if (!strcmp(str, globals[i].name)) {
				snprintf(error_buf, sizeof(error_buf),
					 "%s defined in %s:%d and %s:%d", str,
					 obj->fname, line_no,
					 obj_files[globals[i].obj_index].fname,
					 globals[i].line_no);
				err_type = SALINK_ERROR_MULTIPLE_DEFS;
				return 0;
			}
		}
		if (!strcmp(str, "Main")) {
			main_index = obj_file_count;
			prv_init_out_fnames(obj);
		}
		global = &globals[i];
		strcpy(global->name, str);
		global->obj_index = obj_file_count;
		global->label_index = label_count;
		global->line_no = line_no;
		global_count++;
		itoa(global_count, ibuf, 10);
		(void)specasm_text_print(ibuf, SALINK_VAL_COL + 1,
					 SALINK_FIELD_GLOBALS_ROW,
					 SPECASM_CODE_COLOUR);
		retval = 1;
	} else {
		for (i = obj->label_start; i < label_count; i++) {
			if ((labels[i].type == SALINK_LABEL_TYPE_ALIGN) ||
			    (labels[i].type > SALINK_LABEL_TYPE_EQU_LONG))
				continue;

			str1 = salink_get_label_str_e(labels[i].id,
						      labels[i].type);
			if (err_type != SPECASM_ERROR_OK)
				return 0;

			if (!strcmp(str, str1)) {
				snprintf(error_buf, sizeof(error_buf),
					 "%s multiply defined in %s:%d", str,
					 obj->fname, line_no);
				err_type = SALINK_ERROR_MULTIPLE_DEFS;
				return 0;
			}
		}
	}
	label_count++;

	return retval;
}

static void prv_add_align_e(salink_obj_t *obj, specasm_line_t *line,
			    uint16_t size)
{
	salink_label_t *label;

	if (label_count == MAX_LABELS) {
		snprintf(error_buf, sizeof(error_buf),
			 "Max label limit %d reached", MAX_LABELS);
		err_type = SALINK_ERROR_TOO_MANY_LABELS;
		return;
	}
	label = &labels[label_count++];
	label->type = SALINK_LABEL_TYPE_ALIGN;
	label->id = line->data.op_code[0];
	label->data.off = size;
}

static void prv_add_queued_file_e(const char *base, const char *prefix,
				  specasm_line_t *line)
{
	const char *str;
	uint8_t id;
	int space_needed;
	int prefix_len;
	int base_len;
	char *ptr;
	char *start;
	char *slash;

	if (queued_files == MAX_PENDING_X_FILES) {
		snprintf(error_buf, sizeof(error_buf),
			 "Pending file limit %d reached", MAX_PENDING_X_FILES);
		err_type = SALINK_ERROR_TOO_MANY_FILES;
		return;
	}

	id = line->data.label;
	str = salink_get_label_str_e(id, line->type);

	base_len = 0;
	if (str[0] != '/') {
		slash = strrchr(base, '/');
		if (slash)
			base_len = (slash - base) + 1;
	}

	prefix_len = strlen(prefix);
	space_needed = strlen(str) + prefix_len + base_len;
	if (space_needed > MAX_FNAME) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return;
	}

	ptr = &buf.fname[queued_files++][0];
	start = ptr;
	if (base_len) {
		strncpy(ptr, base, base_len);
		ptr += base_len;
	}
	if (prefix_len) {
		strcpy(ptr, prefix);
		ptr += prefix_len;
	}
	strcpy(ptr, str);
	if (start[space_needed - 2] != '.' &&
	    (start[space_needed - 1] | 32) != 'x') {
		if (space_needed + 2 > MAX_FNAME) {
			err_type = SPECASM_ERROR_BAD_FNAME;
			return;
		}
		start[space_needed] = '.';
		start[space_needed + 1] = 'x';
		start[space_needed + 2] = 0;
	}
}

static void prv_add_equ_label_e(specasm_line_t *line, salink_obj_t *obj,
				uint16_t line_no)
{
	uint8_t type;
	uint8_t *op_code;
	uint8_t is_global;
	salink_global_t *global;
	uint8_t id;
	const char *str;
	salink_label_t *label;

	/*
	 * With local equ expressions we can just store the expression
	 * information in the label itself for later use.  For global
	 * expressions we need to store it in the label name itself,
	 * as a second string.  There's guaranteed to be enough room
	 * for this as the sum of the label and the expression cannot
	 * exceed the max line length.
	 */

	op_code = &line->data.op_code[0];
	type = op_code[0] == SPECASM_LINE_TYPE_SL ? SALINK_LABEL_TYPE_EQU_SHORT
						  : SALINK_LABEL_TYPE_EQU_LONG;
	is_global = prv_add_label_e(obj, 0, type, op_code[1], line_no);
	if (err_type != SPECASM_ERROR_OK)
		return;
	label = &labels[label_count - 1];
	if (!is_global) {
		label->data.equ[0] = op_code[2];
		label->data.equ[1] = op_code[3];
		return;
	}

	global = &globals[global_count - 1];
	id = op_code[3];
	str = salink_get_label_str_e(id, op_code[2]);
	if (err_type != SPECASM_ERROR_OK)
		return;
	strcpy(global->name + strlen(global->name) + 1, str);
	label->type = SALINK_LABEL_TYPE_EQU_GLOBAL;
}

static void prv_parse_obj_e(const char *fname)
{
	uint16_t i;
	specasm_line_t *line;
	salink_obj_t *obj;
	char ibuf[16];
	uint8_t type;
	uint16_t size = 0;

	if (obj_file_count == MAX_FILES) {
		snprintf(error_buf, sizeof(error_buf),
			 "Max file limit %d reached", MAX_FILES);
		err_type = SALINK_ERROR_TOO_MANY_FILES;
		return;
	}

	obj = &obj_files[obj_file_count];

	if (strlen(fname) > MAX_FNAME) {
		err_type = SPECASM_ERROR_STRING_TOO_LONG;
		return;
	}
	strcpy(obj->fname, fname);
	obj->label_start = label_count;

	specasm_load_e(fname);
	if (err_type != SPECASM_ERROR_OK) {
		snprintf(error_buf, sizeof(error_buf), "Can't open %s", fname);
		err_type = SALINK_ERROR_CANT_OPEN;
		return;
	}

	for (i = 0; i < state.lines.num_lines; i++) {
		line = &state.lines.lines[i];
		if ((line->type == SPECASM_LINE_TYPE_LL) ||
		    (line->type == SPECASM_LINE_TYPE_SL)) {
			type = line->type == SPECASM_LINE_TYPE_LL
				   ? SALINK_LABEL_TYPE_LNG
				   : SALINK_LABEL_TYPE_SHORT;
			(void)prv_add_label_e(obj, size, type, line->data.label,
					      i);
		} else if (line->type == SPECASM_LINE_TYPE_EQU) {
			prv_add_equ_label_e(line, obj, i);
		} else if (line->type == SPECASM_LINE_TYPE_ORG) {
			if (got_org) {
				strcpy(error_buf,
				       "Only one org statement allowed");
				err_type = SALINK_ERROR_TOO_MANY_ORGS;
				return;
			}
			got_org = 1;
			start_address = *((uint16_t *)&line->data.op_code[0]);
		} else if (line->type == SPECASM_LINE_TYPE_MAP) {
			map_file = 1;
		} else if (line->type == SPECASM_LINE_TYPE_ALIGN) {
			prv_add_align_e(obj, line, size);
		} else if ((line->type >= SPECASM_LINE_TYPE_INC_SHORT) &&
			   (line->type <= SPECASM_LINE_TYPE_INC_LONG)) {
			prv_add_queued_file_e(obj->fname, "", line);
		} else if ((line->type >= SPECASM_LINE_TYPE_INC_SYS_SHORT) &&
			   (line->type <= SPECASM_LINE_TYPE_INC_SYS_LONG)) {
			prv_add_queued_file_e(fname, "/specasm/", line);
		} else {
			size += specasm_compute_line_size(line);
		}

		if (err_type != SPECASM_ERROR_OK)
			return;
	}

	obj_file_count++;
	obj->label_end = label_count;
	obj->size = size;

	itoa(obj_file_count, ibuf, 10);
	(void)specasm_text_print(ibuf, SALINK_VAL_COL + 1,
				 SALINK_FIELD_FILES_ROW, SPECASM_CODE_COLOUR);
}

static void prv_process_queued_files_e(void)
{
	const char *path;

	while (queued_files > 0) {
		--queued_files;
		path = &buf.fname[queued_files][0];
		prv_parse_obj_e(path);
		if (err_type != SPECASM_ERROR_OK)
			return;
	}
}

static uint8_t prv_order_objects_e(void)
{
	salink_obj_t tmp;
	uint8_t loaded;
	uint8_t i;
	salink_global_t *glob;

	/*
	 * We want to put the object file with the Main label
	 * first.
	 */

	if (main_index == SIZE_MAX) {
		strcpy(error_buf, "No Main label defined");
		err_type = SALINK_ERROR_NO_MAIN;
		return 0;
	}

	loaded = main_index == (obj_file_count - 1);
	if (main_index > 0) {
		memcpy(&tmp, &obj_files[0], sizeof(obj_files[0]));
		memcpy(&obj_files[0], &obj_files[main_index],
		       sizeof(obj_files[0]));
		memcpy(&obj_files[main_index], &tmp, sizeof(obj_files[0]));
		for (i = 0; i < global_count; i++) {
			glob = &globals[i];
			if (glob->obj_index == 0)
				glob->obj_index = main_index;
			else if (glob->obj_index == main_index)
				glob->obj_index = 0;
		}
		main_index = 0;
	}

	return loaded;
}

static void prv_complete_absolutes_e(void)
{
	unsigned int i;
	uint16_t j;
	salink_obj_t *obj;
	salink_label_t *label;
	unsigned int mask;
	unsigned int adjust;
	uint16_t align;
	uint16_t real_off = start_address;

	for (i = 0; i < obj_file_count; i++) {
		obj = &obj_files[i];
		for (j = obj->label_start; j < obj->label_end; j++) {
			label = &labels[j];

			if (label->type > SALINK_LABEL_TYPE_ALIGN)
				continue;

			if (label->type == SALINK_LABEL_TYPE_ALIGN) {
				align = 1 << label->id;
				mask = align - 1;
				adjust = (real_off + label->data.off) & mask;
				if (adjust > 0)
					real_off += align - adjust;
				continue;
			}
			if (label->data.off > 0xffff - real_off) {
				snprintf(error_buf, sizeof(error_buf),
					 "%s past end of memory", obj->fname);
				err_type = SALINK_ERROR_PROGRAM_TOO_BIG;
				return;
			}
			label->data.off += real_off;
		}
		real_off += obj->size;
	}
}

static void prv_evaluate_global_equs_e(void)
{
	uint8_t i;
	salink_obj_t *obj;
	salink_label_t *label;
	salink_global_t *global;

	for (i = 0; i < global_count; i++) {
		global = &globals[i];
		label = &labels[global->label_index];
		obj = &obj_files[global->obj_index];
		if (label->type == SALINK_LABEL_TYPE_EQU_GLOBAL) {
			salink_equ_eval_global_e(obj, global, label, 0);
			if (err_type != SPECASM_ERROR_OK)
				return;
		}
	}
}

static salink_label_t *prv_find_local_label(salink_obj_t *obj, uint8_t lng,
					    uint8_t id)
{
	unsigned int i;
	salink_label_t *label;

	for (i = obj->label_start; i < obj->label_end; i++) {
		label = &labels[i];
		if (lng != label->type)
			continue;
		if (id == label->id)
			return label;
	}

	return NULL;
}

static void prv_unknown_error_label_e(salink_obj_t *obj, const char *str)
{
	snprintf(error_buf, sizeof(error_buf), "%s:Unknown label in equ:%s",
		 obj->fname, str);
	err_type = SALINK_ERROR_UNRESOLVED_LABEL;
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

static salink_global_t *prv_find_global_label_e(salink_obj_t *obj,
						unsigned int line_no,
						uint8_t id, int8_t lng)
{
	const char *str;
	salink_global_t *global;
	unsigned int i;

	str = salink_get_label_str_e(id, lng);
	if (err_type != SPECASM_ERROR_OK)
		return NULL;

	for (i = 0; i < global_count; i++) {
		global = &globals[i];
		if (!strcmp(str, global->name))
			return global;
	}

	snprintf(error_buf, sizeof(error_buf), "%s:%d Unknown: %s", obj->fname,
		 line_no, str);
	err_type = SALINK_ERROR_UNRESOLVED_LABEL;

	return NULL;
}

static void prv_resolve_address_e(salink_obj_t *obj, specasm_line_t *line,
				  unsigned int line_no, uint8_t id,
				  uint16_t *addr, uint8_t lng)
{
	salink_label_t *label;
	salink_global_t *global;

	label = prv_find_local_label(obj, lng, id);
	if (!label) {
		/*
		 * Couldn't find a local address.  Let's check to see whether
		 * it's global.
		 */

		global = prv_find_global_label_e(obj, line_no, id, lng);
		if (err_type != SPECASM_ERROR_OK)
			return;
		label = &labels[global->label_index];
	}

	*addr = label->data.off;
}

static int16_t prv_eval_exp_from_id_e(salink_obj_t *obj, unsigned int line_no,
				      uint8_t id, uint8_t lng)
{
	const char *str;
	int16_t exp;

	str = salink_get_label_str_e(id, lng);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	exp = salink_equ_eval_e(obj, str, line_no);

	if ((err_type != SPECASM_ERROR_OK) && (err_type < SPECASM_MAX_ERRORS)) {
		snprintf(error_buf, sizeof(error_buf), "%s:%d %s", obj->fname,
			 line_no, specasm_error_msg(err_type));
		err_type = SALINK_ERROR_BAD_EXP;
	}

	return exp;
}

static void prv_resolve_relative_address_e(salink_obj_t *obj,
					   specasm_line_t *line, unsigned int i,
					   uint16_t offset)
{
	salink_label_t *label;
	int16_t diff;
	uint8_t addr_type = specasm_line_get_addr_type(line);
	uint8_t lng = addr_type == SPECASM_FLAGS_ADDR_LONG
			  ? SALINK_LABEL_TYPE_LNG
			  : SALINK_LABEL_TYPE_SHORT;

	label = prv_find_local_label(obj, lng, line->data.op_code[1]);
	if (!label) {
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d Unresolved symbol", obj->fname, i);
		err_type = SALINK_ERROR_UNRESOLVED_LABEL;
		return;
	}

	diff = label->data.off - offset;
	if (diff < -126 || diff > 129) {
		snprintf(error_buf, sizeof(error_buf),
			 "%s line %d label too far", obj->fname, i);
		err_type = SALINK_ERROR_JUMP_TOO_FAR;
		return;
	}
	line->data.op_code[1] = (int8_t)(diff - 2);
}

static void prv_write_line_e(specasm_handle_t f, specasm_line_t *line,
			     uint16_t size)
{
	unsigned int i;
	unsigned int to_set;
	uint8_t id;
	const char *data;

	if ((buf_count + size > MAX_BUFFER_SIZE) ||
	    ((buf_count > 0) && (line->type == SPECASM_LINE_TYPE_DS))) {
		specasm_file_write_e(f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			return;
		bin_size += buf_count;
		buf_count = 0;
	}

	if (line->type <= SPECASM_LINE_TYPE_SIMPLE_MAX) {
		for (i = 0; i < size; i++)
			buf.file_buf[buf_count++] = line->data.op_code[i];
		return;
	}

	if (line->type == SPECASM_LINE_TYPE_DS) {
		to_set = size < MAX_BUFFER_SIZE ? size : MAX_BUFFER_SIZE;
		memset(&buf.file_buf[0], line->data.op_code[0], to_set);
		do {
			size -= to_set;
			if (size == 0) {
				buf_count = to_set;
				return;
			}
			specasm_file_write_e(f, buf.file_buf, MAX_BUFFER_SIZE);
			if (err_type != SPECASM_ERROR_OK)
				return;
			bin_size += MAX_BUFFER_SIZE;
			buf_count = 0;
			to_set =
			    size < MAX_BUFFER_SIZE ? size : MAX_BUFFER_SIZE;
		} while (1);
	}

	if ((line->type >= SPECASM_LINE_TYPE_STR_HSH_SHORT) &&
	    (line->type <= SPECASM_LINE_TYPE_STR_AMP_LONG)) {
		size--;
		buf.file_buf[buf_count++] = size;
	}

	if ((line->type >= SPECASM_LINE_TYPE_STR_SIN_SHORT) &&
	    (line->type <= SPECASM_LINE_TYPE_STR_AMP_LONG)) {
		id = line->data.label;
		data = salink_get_label_str_e(id, line->type);
		memcpy(&buf.file_buf[buf_count], data, size);
		buf_count += size;
	}
}

static void prv_label_subtraction_e(salink_obj_t *obj, specasm_line_t *line,
				    unsigned int l, uint8_t id_pos)
{
	uint16_t a;
	uint16_t b;
	uint8_t id1;
	uint8_t id2;
	uint8_t lng;
	uint8_t *op_code = &line->data.op_code[id_pos];

	id1 = op_code[0];
	id2 = op_code[1];
	lng = op_code[2] == SPECASM_FLAGS_ADDR_LONG ? SALINK_LABEL_TYPE_LNG
						    : SALINK_LABEL_TYPE_SHORT;
	prv_resolve_address_e(obj, line, l, id1, &a, lng);
	if (err_type != SPECASM_ERROR_OK)
		return;
	lng =
	    specasm_line_get_addr_type(line) == SPECASM_FLAGS_ADDR_LONG ? 1 : 0;
	prv_resolve_address_e(obj, line, l, id2, &b, lng);
	if (err_type != SPECASM_ERROR_OK)
		return;
	if (b > a) {
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d Negative difference", obj->fname, l);
		err_type = SALINK_ERROR_NEGATIVE_SIZE;
		return;
	}
	*((uint16_t *)&op_code[0]) = a - b;
}

static void prv_label_subtraction_byte_e(salink_obj_t *obj,
					 specasm_line_t *line, unsigned int l,
					 uint8_t id_pos)
{
	uint16_t diff;

	prv_label_subtraction_e(obj, line, l, id_pos);
	if (err_type != SPECASM_ERROR_OK)
		return;

	diff = *((uint16_t *)&line->data.op_code[id_pos]);

	if (diff > 255) {
		snprintf(error_buf, sizeof(error_buf), "%s:%d Too big:%d",
			 obj->fname, l, diff);
		err_type = SALINK_ERROR_SIZE_TOO_BIG;
	}
}

static uint16_t prv_align_e(specasm_handle_t f, uint16_t align)
{
	unsigned int mask = align - 1;
	unsigned int adjust = (start_address + bin_size + buf_count) & mask;

	if (adjust == 0)
		return 0;

	adjust = align - adjust;

	if (buf_count + adjust > MAX_BUFFER_SIZE) {
		specasm_file_write_e(f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		bin_size += buf_count;
		buf_count = 0;
	}
	memset(&buf.file_buf[buf_count], 0, adjust);
	buf_count += adjust;
	return adjust;
}

static int16_t prv_eval_equ_label(specasm_line_t *line, salink_obj_t *obj,
				  unsigned int line_no, uint8_t id)
{
	uint8_t label_type;
	uint8_t lng;

	label_type = specasm_line_get_addr_type(line);
	lng = label_type == SPECASM_FLAGS_ADDR_SHORT ? 0 : 1;
	return prv_eval_exp_from_id_e(obj, line_no, id, lng);
}

static void prv_eval_equ_8bit_e(specasm_line_t *line, salink_obj_t *obj,
				unsigned int line_no, uint8_t loc)
{
	int16_t exp;

	exp = prv_eval_equ_label(line, obj, line_no, line->data.op_code[loc]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if ((exp > 255) || (exp < -128)) {
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d immediate too big :%d", obj->fname, line_no,
			 exp);
		err_type = SALINK_ERROR_SIZE_TOO_BIG;
		return;
	}
	line->data.op_code[loc] = exp;
}

static void prv_eval_equ_16bit_e(specasm_line_t *line, salink_obj_t *obj,
				 unsigned int line_no, uint8_t loc)
{
	int16_t exp;

	exp = prv_eval_equ_label(line, obj, line_no, line->data.op_code[loc]);
	if (err_type != SPECASM_ERROR_OK)
		return;

	*((uint16_t *)&line->data.op_code[loc]) = exp;
}

static void prv_apply_expressions_e(specasm_line_t *line, salink_obj_t *obj,
				    unsigned int line_no)
{
	int16_t exp;
	uint8_t opcode0;

	line->type -= SPECASM_LINE_TYPE_EXP_ADJ;

	switch (line->type) {
	case SPECASM_LINE_TYPE_ADC:
	case SPECASM_LINE_TYPE_ADD:
	case SPECASM_LINE_TYPE_AND:
	case SPECASM_LINE_TYPE_CP:
	case SPECASM_LINE_TYPE_IN:
	case SPECASM_LINE_TYPE_OUT:
	case SPECASM_LINE_TYPE_OR:
	case SPECASM_LINE_TYPE_SBC:
	case SPECASM_LINE_TYPE_SUB:
	case SPECASM_LINE_TYPE_XOR:
		prv_eval_equ_8bit_e(line, obj, line_no, 1);
		break;
	case SPECASM_LINE_TYPE_RST:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[1]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if ((exp > 0x38) || (exp & 7)) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad argument to rst :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		line->data.op_code[0] |= exp;
		break;
	case SPECASM_LINE_TYPE_LD:
		opcode0 = line->data.op_code[0];
		if ((opcode0 & 0xC7) == 0x6) {
			prv_eval_equ_8bit_e(line, obj, line_no, 1);
			break;
		}
		switch (opcode0) {
		case 0x1:
		case 0x11:
		case 0x21:
		case 0x31:
		case 0x2a:
		case 0x3a:
		case 0x22:
		case 0x32:
			prv_eval_equ_16bit_e(line, obj, line_no, 1);
			break;
		case 0xDD:
		case 0xED:
		case 0xFD:
			prv_eval_equ_16bit_e(line, obj, line_no, 2);
			break;
		}
		break;
	case SPECASM_LINE_TYPE_BIT:
	case SPECASM_LINE_TYPE_RES:
	case SPECASM_LINE_TYPE_SET:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[2]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (exp > 7) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad bit position :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		line->data.op_code[1] |= ((uint8_t)exp) << 3;
		break;
	case SPECASM_LINE_TYPE_IM:
		exp = prv_eval_equ_label(line, obj, line_no,
					 line->data.op_code[2]);
		if (err_type != SPECASM_ERROR_OK)
			return;
		if (exp > 2) {
			snprintf(error_buf, sizeof(error_buf),
				 "%s:%d bad arg for im :%d", obj->fname,
				 line_no, exp);
			err_type = SALINK_ERROR_SIZE_TOO_BIG;
			return;
		}
		if (exp == 2)
			line->data.op_code[1] = 0x5e;
		else if (exp == 1)
			line->data.op_code[1] = 0x56;
		break;
	case SPECASM_LINE_TYPE_CALL:
	case SPECASM_LINE_TYPE_JP:
		prv_eval_equ_16bit_e(line, obj, line_no, 1);
		break;
	case SPECASM_LINE_TYPE_DB:
		prv_eval_equ_8bit_e(line, obj, line_no, 0);
		break;
	case SPECASM_LINE_TYPE_DW:
		prv_eval_equ_16bit_e(line, obj, line_no, 0);
		break;
	default:
		snprintf(error_buf, sizeof(error_buf),
			 "%s:%d unexpected expression", obj->fname, line_no);
		err_type = SALINK_ERROR_UNEXPECTED_EXP;
		break;
	}

	specasm_line_set_addr_type(line, SPECASM_FLAGS_ADDR_NUM);
}

static uint16_t prv_link_obj_e(specasm_handle_t f, salink_obj_t *obj,
			       uint16_t offset)
{
	unsigned int i;
	specasm_line_t *line;
	uint16_t size;
	uint8_t addr_type;
	uint8_t id_pos;
	uint8_t id;
	uint8_t lng;
	uint16_t *addr;

	for (i = 0; i < state.lines.num_lines; i++) {
		id_pos = 1;
		line = &state.lines.lines[i];

		if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
			prv_apply_expressions_e(line, obj, i);

		switch (line->type) {
		case SPECASM_LINE_TYPE_ALIGN:
			offset += prv_align_e(f, 1 << line->data.op_code[0]);
			break;
		case SPECASM_LINE_TYPE_DW:
		case SPECASM_LINE_TYPE_CALL:
		case SPECASM_LINE_TYPE_JP:
		case SPECASM_LINE_TYPE_LD:
			addr_type = specasm_line_get_addr_type(line);
			if ((addr_type == SPECASM_FLAGS_ADDR_SHORT) ||
			    (addr_type == SPECASM_FLAGS_ADDR_LONG)) {
				/*
				 * For all these instructions the id is
				 * in the penultimate byte.
				 */
				id_pos = specasm_line_get_size(line) - 1;
				id = line->data.op_code[id_pos];
				addr = (uint16_t *)&line->data.op_code[id_pos];
				lng = addr_type == SPECASM_FLAGS_ADDR_LONG ? 1
									   : 0;
				prv_resolve_address_e(obj, line, i, id, addr,
						      lng);
			}
			break;
		case SPECASM_LINE_TYPE_DW_SUB:
			id_pos = 0;
		case SPECASM_LINE_TYPE_LD_IMM_16_SUB:
			prv_label_subtraction_e(obj, line, i, id_pos);
			break;
		case SPECASM_LINE_TYPE_DB_SUB:
			id_pos = 0;
		case SPECASM_LINE_TYPE_LD_IMM_8_SUB:
			prv_label_subtraction_byte_e(obj, line, i, id_pos);
			break;
		case SPECASM_LINE_TYPE_JR:
		case SPECASM_LINE_TYPE_DJNZ:
			prv_resolve_relative_address_e(obj, line, i, offset);
			break;
		default:
			break;
		}
		if (err_type != SPECASM_ERROR_OK)
			return 0;

		size = specasm_compute_line_size(line);
		if (size > 0) {
			prv_write_line_e(f, line, size);
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			offset += size;
		}
	}

	return offset;
}

static void prv_link_e(uint8_t main_loaded)
{
	char ibuf[16];
	unsigned int i;
	specasm_handle_t f;
	uint16_t offset = start_address;
	salink_obj_t *obj = &obj_files[0];

	f = specasm_file_wopen_e(image_name);
	if (err_type != SPECASM_ERROR_OK)
		return;
	for (i = 0; i < obj_file_count; i++) {
		obj = &obj_files[i];
		if (i > 0 || !main_loaded) {
			specasm_load_e(obj->fname);
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
		}
		offset = prv_link_obj_e(f, obj, offset);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	if (buf_count > 0) {
		specasm_file_write_e(f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
		bin_size += buf_count;
	}

	if (bin_size > 0xffff - start_address) {
		snprintf(error_buf, sizeof(error_buf), "%s past end of memory",
			 obj->fname);
		err_type = SALINK_ERROR_PROGRAM_TOO_BIG;
		goto on_error;
	}

	itoa(bin_size, ibuf, 10);
	(void)specasm_text_print(ibuf, SALINK_VAL_COL + 1,
				 SALINK_FIELD_SIZE_ROW, SPECASM_CODE_COLOUR);

	specasm_file_close_e(f);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	return;

on_error:

	specasm_file_close_e(f);
	specasm_remove_file(image_name);

	return;
}

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

static void prv_write_map_e(void)
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

static void prv_salink_e(void)
{
	uint8_t main_loaded;
	char ibuf[16];

	specasm_dir_t dir = specasm_opendir_e(".");
	if (err_type != SPECASM_ERROR_OK) {
		strcpy(error_buf, "Failed to read directory");
		err_type = SALINK_ERROR_READDIR;
		return;
	}
	while (specasm_readdir(dir, &dirent)) {
		if (!prv_check_file(specasm_getdirname(dirent)))
			continue;
		prv_parse_obj_e(specasm_getdirname(dirent));
		if (err_type != SPECASM_ERROR_OK) {
			specasm_closedir(dir);
			return;
		}
	}
	specasm_closedir(dir);

	prv_process_queued_files_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (obj_file_count == 0)
		return;

	itoa(start_address, ibuf, 16);
	(void)specasm_text_print(ibuf, SALINK_VAL_COL + 1,
				 SALINK_FIELD_STARTADDR_ROW,
				 SPECASM_CODE_COLOUR);

	main_loaded = prv_order_objects_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_complete_absolutes_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_evaluate_global_equs_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	specasm_remove_file(image_name);
	specasm_remove_file(map_name);

	prv_link_e(main_loaded);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (map_file) {
		(void)specasm_text_print(map_name, SALINK_VAL_COL,
					 SALINK_FIELD_MAP_ROW,
					 SPECASM_CODE_COLOUR);
		prv_write_map_e();
	} else {
		(void)specasm_text_print("None", SALINK_VAL_COL,
					 SALINK_FIELD_MAP_ROW,
					 SPECASM_CODE_COLOUR);
	}
}

static void prv_setup_screen(void)
{
	unsigned int i;

	specasm_cls(SPECASM_SALINK_BACKGROUND);
	specasm_border(SPECASM_SALINK_BORDER);
	(void)specasm_text_print("            SALINK " SPECASM_VERSION_STR
				 "           ",
				 0, 0, SPECASM_HEADER_COLOUR);

	(void)specasm_text_print("Name", SALINK_FIELD_COL,
				 SALINK_FIELD_NAME_ROW, SPECASM_STATUS_COLOUR);
	(void)specasm_text_print("Start Address", SALINK_FIELD_COL,
				 SALINK_FIELD_STARTADDR_ROW,
				 SPECASM_STATUS_COLOUR);
	(void)specasm_text_print("Source Files", SALINK_FIELD_COL,
				 SALINK_FIELD_FILES_ROW, SPECASM_STATUS_COLOUR);
	(void)specasm_text_print("Globals", SALINK_FIELD_COL,
				 SALINK_FIELD_GLOBALS_ROW,
				 SPECASM_STATUS_COLOUR);
	(void)specasm_text_print("Size", SALINK_FIELD_COL,
				 SALINK_FIELD_SIZE_ROW, SPECASM_STATUS_COLOUR);
	(void)specasm_text_print("Map file", SALINK_FIELD_COL,
				 SALINK_FIELD_MAP_ROW, SPECASM_STATUS_COLOUR);

	for (i = SALINK_FIELD_NAME_ROW; i <= SALINK_FIELD_MAX_ROW; i += 2) {
		(void)specasm_text_print("            ", SALINK_VAL_COL, i,
					 SPECASM_CODE_COLOUR);
	}
}

int main(int argc, char *argv[])
{
	const char *err_str;
	int err_buf_len;
	uint16_t last_line = SALINK_STATUS_ROW;
	int retval = 0;

	prv_setup_screen();

	prv_salink_e();
	if (err_type != SPECASM_ERROR_OK) {
		if (err_type >= SPECASM_MAX_ERRORS)
			err_str = error_buf;
		else
			err_str = specasm_error_msg(err_type);
		err_buf_len = strlen(err_str);
		do {
			(void)specasm_text_print(err_str, 0, last_line,
						 SPECASM_ERROR_COLOUR);
			if (err_buf_len < SPECASM_LINE_MAX_LEN)
				break;
			++last_line;
			err_str = &err_str[SPECASM_LINE_MAX_LEN];
			err_buf_len -= SPECASM_LINE_MAX_LEN;
		} while (1);
		retval = 1;
	} else {
		(void)specasm_text_print("Link succeeded", 0, SALINK_STATUS_ROW,
					 SPECASM_SUCCESS_COLOUR);
		++last_line;
	}
	specasm_screen_flush(last_line + 2);

	return retval;
}
