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

#include "line.h"

#define SPECASM_NEXT_PARSE_BANK (43 << 1)
#define SPECASM_NEXT_DUMP_BANK (44 << 1)
#define SPECASM_NEXT_EDITOR_BANK (45 << 1)
#define SPECASM_NEXT_CLIP_BANK (46 << 1)

#ifdef UNITTESTS
#define SPECASM_NEXT_UNIT_BANK (45 << 1)
#endif

extern unsigned char _z_page_table[];

typedef struct specasm_opcode_t_ specasm_opcode_t;

char *specasm_get_long_imm_banked_e(const char *str, long *val, uint8_t *flags);
void specasm_append_empty_line_banked_e(void);
void specasm_delete_lines_banked(unsigned int start, unsigned int end);
void specasm_insert_lines_banked_e(unsigned int l, unsigned int count);
void specasm_parse_line_banked_e(unsigned int l, const char *str);
uint8_t specasm_parse_mnemomic_banked_e(const char *str, uint8_t i,
					specasm_line_t *line);
void specasm_init_dump_table_banked(void);
uint8_t specasm_dump_opcode_banked_e(const specasm_line_t *line, char *buf);
void specasm_clip_reset_banked(void);
void specasm_clip_add_line_banked_e(const char *line);
uint16_t specasm_clip_get_line_banked(uint16_t ptr, char *buffer);
uint16_t specasm_clip_get_line_count_banked(void);
void specasm_format_line_banked_e(char *buf, unsigned int l);

void specasm_draw_status_banked(void);
void specasm_handle_key_press_banked(uint8_t k);
void specasm_editor_reset_banked(void);
void specasm_editor_preload_banked(const char *fname);

#ifdef UNITTESTS
void specasm_peer_write_state_banked_e(const char *fname, uint16_t checksum);
uint16_t specasm_peer_read_state_banked_e(const char *fname);
#endif

char *specasm_get_long_imm_e(const char *str, long *val, uint8_t *flags)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();
	char *e;

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	e = specasm_get_long_imm_banked_e(str, val, flags);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}


void specasm_append_empty_line_e(void)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	specasm_append_empty_line_banked_e();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_delete_lines(unsigned int start, unsigned int end)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	specasm_delete_lines_banked(start, end);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_insert_lines_e(unsigned int l, unsigned int count)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	specasm_insert_lines_banked_e(l, count);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_parse_line_e(unsigned int l, const char *str)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	specasm_parse_line_banked_e(l, str);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

/** EXPORT Needed by tests**/
uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line)
{
	uint8_t e;
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_PARSE_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_PARSE_BANK+1]);
	e = specasm_parse_mnemomic_banked_e(str, i, line);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}

/** EXPORT Needed by tests**/
void specasm_init_dump_table(void)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_DUMP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_DUMP_BANK+1]);
	specasm_init_dump_table_banked();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

/** EXPORT Needed by tests**/
uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf)
{
	uint8_t e;
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_DUMP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_DUMP_BANK+1]);
	e = specasm_dump_opcode_banked_e(line, buf);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}

/* EXPORT */
void specasm_format_line_e(char *buf, unsigned int l)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_DUMP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_DUMP_BANK+1]);
	specasm_format_line_banked_e(buf, l);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_clip_reset(void)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_CLIP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_CLIP_BANK+1]);
	specasm_clip_reset_banked();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_clip_add_line_e(const char *line)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_CLIP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_CLIP_BANK+1]);
	specasm_clip_add_line_banked_e(line);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

uint16_t specasm_clip_get_line(uint16_t ptr, char *buffer)
{
	uint16_t e;
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_CLIP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_CLIP_BANK+1]);
	e = specasm_clip_get_line_banked(ptr, buffer);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}

uint16_t specasm_clip_get_line_count(void)
{
	uint16_t e;
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_CLIP_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_CLIP_BANK+1]);
	e = specasm_clip_get_line_count_banked();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}

#ifndef UNITTESTS
void specasm_draw_status(void)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_EDITOR_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_EDITOR_BANK+1]);
	specasm_draw_status_banked();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_handle_key_press(uint8_t k)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_EDITOR_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_EDITOR_BANK+1]);
	specasm_handle_key_press_banked(k);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_editor_reset(void)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_EDITOR_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_EDITOR_BANK+1]);
	specasm_editor_reset_banked();
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

void specasm_editor_preload(const char *fname)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_EDITOR_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_EDITOR_BANK+1]);
	specasm_editor_preload_banked(fname);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}
#else
void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_UNIT_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_UNIT_BANK+1]);
	specasm_peer_write_state_banked_e(fname, checksum);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);
}

uint16_t specasm_peer_read_state_e(const char *fname)
{
	uint16_t e;
	unsigned char bank_l = ZXN_READ_MMU6();
	unsigned char bank_h = ZXN_READ_MMU7();

	ZXN_WRITE_MMU6(_z_page_table[SPECASM_NEXT_UNIT_BANK]);
	ZXN_WRITE_MMU7(_z_page_table[SPECASM_NEXT_UNIT_BANK+1]);
	e = specasm_peer_read_state_banked_e(fname);
	ZXN_WRITE_MMU7(bank_h);
	ZXN_WRITE_MMU6(bank_l);

	return e;
}
#endif
