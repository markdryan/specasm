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
#include "map.h"
#include "peer.h"
#include "queued_files.h"
#include "salink.h"
#include "state_base.h"

static uint8_t got_org;
static uint8_t map_file;
static size_t label_count;
static uint8_t main_index = 0xff;
static uint16_t start_address = 0x8000;

static const char blank_field[] = "            ";

static uint8_t prv_check_file(const char *fname)
{
	char *period;
	uint8_t is_test_file;
	uint8_t is_x_file;

	period = strchr(fname, '.');

	if (!period || !period[1] || period[2])
		return 0;

	is_test_file = (period[1] == 't') || (period[1] == 'T');
	is_x_file = (period[1] == 'x') || (period[1] == 'X');

	if (link_mode == SALINK_MODE_LINK) {
		if (is_test_file)
			got_test = 1;
		return is_x_file;
	}

	return is_test_file || is_x_file;
}

static void prv_init_out_fnames(salink_obj_t *obj)
{
	uint8_t name_len;
	unsigned int i;
	char *ptr;
	char back_ch = 0;

	for (i = 0; i < MAX_FNAME; i++) {
		if (obj->fname[i] == '.')
			break;
		image_name[i] = obj->fname[i];
	}
	if (link_mode == SALINK_MODE_TEST) {
		if (i + 4 > MAX_FNAME)
			i = MAX_FNAME - 4;
		ptr = &image_name[i];
		ptr[0] = '.';
		ptr[1] = 't';
		ptr[2] = 's';
		ptr[3] = 't';
		i += 4;
	}
	image_name[i] = 0;

	name_len = strlen(image_name);
	if (name_len > sizeof(blank_field) - 2) {
		back_ch = image_name[sizeof(blank_field) - 2];
		image_name[sizeof(blank_field) - 2] = 0;
	}

	(void)specasm_text_print(image_name, SALINK_VAL_COL + 1,
				 SALINK_FIELD_NAME_ROW, SPECASM_CODE_COLOUR);

	if (back_ch)
		image_name[sizeof(blank_field) - 2] = back_ch;

	strcpy(map_name, image_name);
	if (link_mode == SALINK_MODE_LINK) {
		if (i + 4 > MAX_FNAME)
			i = MAX_FNAME - 4;
		ptr = &map_name[i];
		ptr[0] = '.';
		ptr[1] = 'm';
		ptr[2] = 'a';
		ptr[3] = 'p';
	} else {
		ptr = &map_name[i - 3];
		ptr[0] = 't';
		ptr[1] = 'm';
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

static void prv_flush_write_buf_e(specasm_handle_t out_f)
{
	if (buf_count > 0) {
		specasm_file_write_e(out_f, buf.file_buf, buf_count);
		if (err_type != SPECASM_ERROR_OK)
			return;
		bin_size += buf_count;
		buf_count = 0;
	}
}

static uint16_t prv_write_bin_file_e(specasm_handle_t out_f,
				     specasm_line_t *line)
{
	specasm_handle_t in_f;
	size_t read;
	specasm_error_t err;
	const char *fname;
	uint16_t file_size = 0;

	/*
	 * Flush the buffer to make things a bit simpler.
	 */

	prv_flush_write_buf_e(out_f);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	fname = salink_get_label_str_e(line->data.label, line->type);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	in_f = specasm_file_ropen_e(fname);
	if (err_type != SPECASM_ERROR_OK) {
		snprintf(error_buf, sizeof(error_buf), "Can't open %s", fname);
		err_type = SALINK_ERROR_CANT_OPEN;
		return 0;
	}

	do {
		read = specasm_file_read_e(in_f, buf.file_buf, MAX_BUFFER_SIZE);
		if (err_type != SPECASM_ERROR_OK)
			break;

		file_size += read;
		if (read < MAX_BUFFER_SIZE) {
			buf_count = read;
			break;
		}

		buf_count = MAX_BUFFER_SIZE;
		prv_flush_write_buf_e(out_f);
		if (err_type != SPECASM_ERROR_OK)
			break;
	} while (1);

	err = err_type;
	specasm_file_close_e(in_f);
	err_type = err;

	return file_size;
}

static void prv_to_zx81_str(char *str, uint8_t len)
{
	uint8_t i;

	for (i = 0; i < len; i++)
		str[i] = salink_to_zx81_char(str[i]);
}

static const uint8_t zx81_two_byte_opcodes[] = {
    0xce, 0xc6, 0xe6, 0xfe, 0x36, 0xf6, 0xde, 0xd6,
    0xee, 0x3e, 0x06, 0x0e, 0x16, 0x1e, 0x26, 0x2e};

static const uint8_t zx81_three_byte_opcodes[] = {
    0x1, 0x11, 0x31, 0x21,
};

static void prv_zx81_patch_opcode(specasm_line_t *line)
{
	uint8_t i;
	char ch;
	uint8_t oc;
	const uint8_t *ptr;
	uint8_t size;

	/*
	 * This is very, very fiddly.  We need to make sure we only update
	 * instructions with characters, which is easier said than done.
	 * We don't want to update instructions with numbers, labels or
	 * expressions.  There's no consistent API to detect this so we
	 * need to check on an instruction by instruction basis and write
	 * lots of tests.
	 */

	if (specasm_line_get_size(line) == 0)
		return;

	/*
	 * Check for the following 4 byte instructions.
	 *
	 * ld (ix+1), 'A'
	 * ld (iy+1), 'A'
	 * ld ix, 'A'
	 * ld iy, 'A'
	 */

	if (specasm_line_get_size(line) == 3) {
		oc = line->data.op_code[0];
		if ((oc != 0xfd) && (oc != 0xdd))
			return;
		oc = line->data.op_code[1];
		if (oc == 0x36) {
			if (specasm_line_get_format2(line) !=
			    SPECASM_FLAGS_NUM_CHAR)
				return;
			ch = (char)line->data.op_code[3];
			line->data.op_code[3] = (uint8_t)salink_to_zx81_char(ch);
		} else if (oc == 0x21) {
			if (specasm_line_get_addr_type(line))
				return;
			if (specasm_line_get_format(line) !=
			    SPECASM_FLAGS_NUM_CHAR)
				return;
			ch = (char)line->data.op_code[2];
			line->data.op_code[2] = (uint8_t)salink_to_zx81_char(ch);
		}
		return;
	}

	if (specasm_line_get_format(line) != SPECASM_FLAGS_NUM_CHAR)
		return;

	/*
	 * Character substitution is performed on the following
	 * two byte instructions.
	 *
	 * adc a, 'A'
	 * add a, 'A'
	 * and 'A'
	 * cp  'A'
	 * ld (hl), 'A'
	 * ld a, 'A'
	 * ld b, 'A'
	 * ld  c , 'A'
	 * ld d, 'A'
	 * ld e, 'A'
	 * ld h, 'A'
	 * ld l, 'A'
	 * or 'A'
	 * sbc a, 'A'
	 * sub 'A'
	 * xor 'A'
	 */

	/*
	 * And on the following 3 byte sequences
	 *
	 * ld bc, 'A',
	 * ld de, 'A'
	 * ld hl, 'A'
	 * ld sp, 'A'
	 */

	if (specasm_line_get_size(line) == 1) {
		ptr = zx81_two_byte_opcodes;
		size = sizeof(zx81_two_byte_opcodes);
	} else {
		/*
		 * Note, I would say that there's a bug here.
		 * The addr_type should be NUM but it's not set
		 * at all for LD BC,DE,HL, imm16.
		 */

		if (specasm_line_get_addr_type(line))
			return;
		ptr = zx81_three_byte_opcodes;
		size = sizeof(zx81_three_byte_opcodes);
	}

	for (i = 0; i < size; i++)
		if (line->data.op_code[0] == ptr[i]) {
			ch = (char)line->data.op_code[1];
			line->data.op_code[1] = (uint8_t)salink_to_zx81_char(ch);
			return;
		}
}

static void prv_zx81_patch_db_dw(specasm_line_t *line)
{
	uint8_t i;
	uint8_t step;
	char ch;

	if (specasm_line_get_format(line) != SPECASM_FLAGS_NUM_CHAR)
		return;

	if (line->type == SPECASM_LINE_TYPE_DB) {
		step = 1;
	} else {

		/*
		 * DWs can have labels in them as well as expressions so we
		 * need to be careful here.
		 */

		if (specasm_line_get_addr_type(line))
			return;
		step = 2;
	}

	for (i = 0; i < specasm_line_get_size(line) + 1; i += step) {
		ch = (char)line->data.op_code[i];
		line->data.op_code[i] = (uint8_t)salink_to_zx81_char(ch);
	}
}

static void prv_write_line_e(specasm_handle_t f, specasm_line_t *line,
			     uint16_t size)
{
	unsigned int i;
	unsigned int to_set;
	uint8_t id;
	uint8_t byt;
	const char *data;

	if ((buf_count + size > MAX_BUFFER_SIZE) ||
	    (line->type == SPECASM_LINE_TYPE_DS)) {
		prv_flush_write_buf_e(f);
		if (err_type != SPECASM_ERROR_OK)
			return;
	}

	if (line->type <= SPECASM_LINE_TYPE_SIMPLE_MAX) {
		for (i = 0; i < size; i++)
			buf.file_buf[buf_count++] = line->data.op_code[i];
		return;
	}

	if (line->type == SPECASM_LINE_TYPE_DS) {
		byt = line->data.op_code[0];
		if (got_zx81 &&
		    (specasm_line_get_format(line) == SPECASM_FLAGS_NUM_CHAR))
			byt = salink_to_zx81_char(byt);
		to_set = size < MAX_BUFFER_SIZE ? size : MAX_BUFFER_SIZE;
		memset(&buf.file_buf[0], byt, to_set);
		do {
			size -= to_set;
			if (size == 0) {
				buf_count = to_set;
				return;
			}
			buf_count = MAX_BUFFER_SIZE;
			prv_flush_write_buf_e(f);
			if (err_type != SPECASM_ERROR_OK)
				return;
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
		if (got_zx81)
			prv_to_zx81_str(&buf.file_buf[buf_count], size);
		buf_count += size;
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

static void prv_stat_fname_e(const char *fname, specasm_stat_t *stat_buf)
{
	specasm_handle_t in_f;
	specasm_error_t err;

	in_f = specasm_file_ropen_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	specasm_file_stat_e(in_f, stat_buf);
	err = err_type;
	specasm_file_close_e(in_f);
	err_type = err;
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

static uint16_t prv_inc_bin_size_e(specasm_line_t *line, uint16_t size)
{
	const char *fname;
	specasm_stat_t stat_buf;
	uint32_t bin_size;
	uint32_t new_size;

	fname = salink_get_label_str_e(line->data.label, line->type);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	prv_stat_fname_e(fname, &stat_buf);
	if (err_type != SPECASM_ERROR_OK) {
		snprintf(error_buf, sizeof(error_buf), "Can't open or stat %s",
			 fname);
		err_type = SALINK_ERROR_CANT_OPEN;
		return 0;
	}

	bin_size = specasm_get_file_size(&stat_buf);
	new_size = bin_size + size;

	if ((new_size < bin_size) || (bin_size > 0xffff)) {
		snprintf(error_buf, sizeof(error_buf), "no room for %s", fname);
		err_type = SALINK_ERROR_PROGRAM_TOO_BIG;
		return 0;
	}

	return (uint16_t)new_size;
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

	specasm_load_e(obj->fname);
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
		} else if (line->type == SPECASM_LINE_TYPE_ZX81) {
			got_zx81 = 1;
		} else if (line->type == SPECASM_LINE_TYPE_MAP) {
			map_file = 1;
		} else if (line->type == SPECASM_LINE_TYPE_ALIGN) {
			prv_add_align_e(obj, line, size);
		} else if ((line->type >= SPECASM_LINE_TYPE_INC_SHORT) &&
			   (line->type <= SPECASM_LINE_TYPE_INC_LONG)) {
			salink_add_queued_file_e(obj->fname, empty_str, line);
		} else if ((line->type >= SPECASM_LINE_TYPE_INC_SYS_SHORT) &&
			   (line->type <= SPECASM_LINE_TYPE_INC_SYS_LONG)) {
			salink_add_queued_file_e(obj->fname, specasm_str, line);
		} else if ((line->type == SPECASM_LINE_TYPE_INC_BIN_SHORT) ||
			   (line->type == SPECASM_LINE_TYPE_INC_BIN_LONG)) {
			size = prv_inc_bin_size_e(line, size);
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

static int prv_obj_file_cmp(const void *a, const void *b)
{
	const uint8_t *a_i = (const uint8_t *)a;
	const uint8_t *b_i = (const uint8_t *)b;

	return strcmp(obj_files[*a_i].fname, obj_files[*b_i].fname);
}

static uint8_t prv_order_objects_e(void)
{
	uint8_t i;

	/*
	 * We want to put the object file with the Main label
	 * first.  The remaining object files are placed in
	 * ascending order of their file names before being
	 * written out to the final binary.
	 */

	for (i = 0; i < obj_file_count; i++)
		obj_files_order[i] = i;

	if (main_index != 0) {
		obj_files_order[main_index] = 0;
		obj_files_order[0] = main_index;
	}

	if (obj_file_count > 2) {
		qsort(&obj_files_order[1], obj_file_count - 1, sizeof(uint8_t),
		      prv_obj_file_cmp);
	}

	return main_index == (obj_file_count - 1);
}

static void prv_check_duplicate_objs_e(void)
{
	uint8_t i;
	const char *fname1;

	/*
	 * It's not possible to include the main file twice as we'll get an
	 * error telling us that the .Main label has been defined multiple
	 * times.  Probably not worth the extra code to detect this case and
	 * report this error instead.
	 */

	/*
	 * We rely on the fact that all but the first object file are sorted
	 * in ascending alphabetical order, so we only need obj_file_count - 2
	 * comparisons.
	 */

	for (i = 1; i < obj_file_count - 1; i++) {
		fname1 = obj_files[obj_files_order[i]].fname;
		if (!strcmp(fname1, obj_files[obj_files_order[i + 1]].fname)) {
			err_type = SALINK_ERROR_DUP_OBJ_FILE;
			sprintf(error_buf, "%s included twice!", fname1);
			return;
		}
	}
}

static void prv_complete_absolutes_e(void)
{
	uint8_t i;
	uint16_t j;
	salink_obj_t *obj;
	salink_label_t *label;
	unsigned int mask;
	unsigned int adjust;
	uint16_t align;
	uint16_t real_off = start_address;

	for (i = 0; i < obj_file_count; i++) {
		obj = &obj_files[obj_files_order[i]];
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

		/*
		 * We want to do this here as we don't want to accidentally
		 * replace numbers in instructions with expressions that cannot
		 * be characters.  We can't do it after the expressions have
		 * been applied as lines that started out as expressions don't
		 * set the format bit correctly and we can't change it now.
		 */

		if (got_zx81) {
			if (line->type < SPECASM_LINE_TYPE_DB)
				prv_zx81_patch_opcode(line);
			else if ((line->type == SPECASM_LINE_TYPE_DB) ||
				 (line->type == SPECASM_LINE_TYPE_DW))
				prv_zx81_patch_db_dw(line);
		}

		if (line->type >= SPECASM_LINE_TYPE_EXP_ADJ)
			salink_apply_expressions_e(line, obj, i);

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
		case SPECASM_LINE_TYPE_LD_IMM_16_SUB:
		case SPECASM_LINE_TYPE_DB_SUB:
		case SPECASM_LINE_TYPE_LD_IMM_8_SUB:
			snprintf(error_buf, sizeof(error_buf),
				 "%s too old. Load/save in Specasm",
				 obj->fname);
			err_type = SALINK_ERROR_X_FILE_TOO_OLD;
			break;
		case SPECASM_LINE_TYPE_JR:
		case SPECASM_LINE_TYPE_DJNZ:
			prv_resolve_relative_address_e(obj, line, i, offset);
			break;
		case SPECASM_LINE_TYPE_INC_BIN_SHORT:
		case SPECASM_LINE_TYPE_INC_BIN_LONG:
			offset += prv_write_bin_file_e(f, line);
			continue;
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

/*
 * Format of the Test Table
 *
 * - 1 or more variable sized entries.  Each entry has a
 *   - 2 byte address
 *   - a null terminated string containing the test name
 * - 2 byte absolute address pointing to the start of the Test Table
 * - 1 byte containing the number of tests.
 *
 * The BASIC test harnesses can locate the table by loading the last two
 * words from the file.
 */

static void prv_write_test_table_e(specasm_handle_t f)
{
	unsigned int i;
	char *period;
	uint8_t name_len;
	salink_global_t *global;
	uint16_t jump_table_start;
	const char *name_ptr;
	uint8_t tests = 0;

	jump_table_start = buf_count + start_address + bin_size;

	for (i = 0; i < global_count; i++) {
		global = &globals[i];
		if (strncmp(global->name, "Test", 4))
			continue;
		period = strchr(obj_files[global->obj_index].fname, '.');
		if (!period || (period[1] != 't' && period[1] != 'T'))
			continue;

		/*
		 * Let's not bother writing the Test prefix.
		 */
		name_ptr = &global->name[4];
		name_len = strlen(name_ptr) + 1;
		if (buf_count + name_len + sizeof(uint16_t) > MAX_BUFFER_SIZE) {
			prv_flush_write_buf_e(f);
			if (err_type != SPECASM_ERROR_OK)
				return;
		}
		memcpy(&buf.file_buf[buf_count],
		       &labels[global->label_index].data.off, sizeof(uint16_t));
		buf_count += 2;
		memcpy(&buf.file_buf[buf_count], name_ptr, name_len);
		buf_count += name_len;
		tests++;
	}

	if (tests == 0) {
		snprintf(error_buf, sizeof(error_buf), "No tests!");
		err_type = SALINK_ERROR_NO_TESTS;
		return;
	}

	if (buf_count + sizeof(uint16_t) + sizeof(uint8_t) > MAX_BUFFER_SIZE) {
		prv_flush_write_buf_e(f);
		if (err_type != SPECASM_ERROR_OK)
			return;
	}
	memcpy(&buf.file_buf[buf_count], &jump_table_start, sizeof(uint16_t));
	buf_count += 2;
	buf.file_buf[buf_count++] = tests;
}

static void prv_link_e(uint8_t main_loaded)
{
	char ibuf[16];
	uint8_t i;
	specasm_handle_t f;
	uint16_t offset = start_address;
	salink_obj_t *obj = &obj_files[main_index];

	f = specasm_file_wopen_e(image_name);
	if (err_type != SPECASM_ERROR_OK)
		return;
	for (i = 0; i < obj_file_count; i++) {
		obj = &obj_files[obj_files_order[i]];
		if (i > 0 || !main_loaded) {
			specasm_load_e(obj->fname);
			if (err_type != SPECASM_ERROR_OK)
				goto on_error;
		}
		offset = prv_link_obj_e(f, obj, offset);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	if (link_mode == SALINK_MODE_TEST) {
		prv_write_test_table_e(f);
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

static void prv_salink_e(void)
{
	uint8_t main_loaded;
	char ibuf[16];
	specasm_dirent_t dirent;
	uint8_t name_len;
	char back_ch = 0;

	specasm_dir_t dir = specasm_opendir_e(".");
	if (err_type != SPECASM_ERROR_OK) {
		strcpy(error_buf, "Failed to read directory");
		err_type = SALINK_ERROR_READDIR;
		return;
	}
	while (specasm_readdir(dir, &dirent)) {
		if (specasm_isdirent_dir(dirent))
			continue;
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

	/*
	 * There has to be a Main label somewhere.  This can be in a .t file
	 * or a .x file but we need to have one.
	 */

	if (main_index == 0xFF) {
		if (((link_mode == SALINK_MODE_LINK) && (!got_test)) ||
		    (link_mode == SALINK_MODE_TEST)) {
			strcpy(error_buf, "No Main label defined");
			err_type = SALINK_ERROR_NO_MAIN;
		}
		return;
	}

	/*
	 * Default to org 16514 on ZX81 if start address not explcitily
	 * provided.
	 */

	if (got_zx81 && !got_org)
		start_address = 16514;

	itoa(start_address, ibuf, 16);
	(void)specasm_text_print(ibuf, SALINK_VAL_COL + 1,
				 SALINK_FIELD_STARTADDR_ROW,
				 SPECASM_CODE_COLOUR);

	main_loaded = prv_order_objects_e();
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_check_duplicate_objs_e();
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
		name_len = strlen(map_name);
		if (name_len > sizeof(blank_field) - 2) {
			back_ch = map_name[sizeof(blank_field) - 2];
			map_name[sizeof(blank_field) - 2] = 0;
		}

		(void)specasm_text_print(map_name, SALINK_VAL_COL + 1,
					 SALINK_FIELD_MAP_ROW,
					 SPECASM_CODE_COLOUR);
		if (back_ch)
			map_name[sizeof(blank_field) - 2] = back_ch;

		specasm_write_map_e();
	} else {
		(void)specasm_text_print("None", SALINK_VAL_COL + 1,
					 SALINK_FIELD_MAP_ROW,
					 SPECASM_CODE_COLOUR);
	}
}

static void prv_setup_screen(void)
{
	unsigned int i;

	specasm_cls(SPECASM_SALINK_BACKGROUND);
	specasm_border(SPECASM_SALINK_BORDER);

#ifdef SPECASM_TARGET_NEXT_OPCODES
	(void)specasm_text_print("           SALINK " SPECASM_VERSION_STR
#else
	(void)specasm_text_print("            SALINK " SPECASM_VERSION_STR
#endif
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
		(void)specasm_text_print(blank_field, SALINK_VAL_COL, i,
					 SPECASM_CODE_COLOUR);
	}
}

static int prv_salink_pass_e(void)
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
	} else if ((link_mode == SALINK_MODE_LINK) && (main_index == 0xFF)) {
		(void)specasm_text_print("Skipping main binary", 0,
					 SALINK_STATUS_ROW,
					 SPECASM_SUCCESS_COLOUR);
		++last_line;
	} else {
		(void)specasm_text_print("Link succeeded", 0, SALINK_STATUS_ROW,
					 SPECASM_SUCCESS_COLOUR);
		++last_line;
	}
	specasm_screen_flush(last_line + 2);

	return retval;
}

#ifdef SPECASM_NEXT_BANKED
int salink_link_banked_e(void)
#else
int salink_link_e(void)
#endif
{
	int retval;

	for (link_mode = SALINK_MODE_LINK; link_mode < SALINK_MODE_MAX;
	     link_mode++) {
		retval = prv_salink_pass_e();
		if (retval || !got_test)
			break;

		specasm_sleep_ms(500);

		/*
		 * Reset state.
		 */

		queued_files = 0;
		label_count = 0;
		start_address = 0x8000;
		got_org = 0;
		global_count = 0;
		obj_file_count = 0;
		bin_size = 0;
		buf_count = 0;
	}

	return retval;
}
