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

#include <arch/zx.h>
#include <stdint.h>

#include "line.h"

static uint8_t banks[] = {
	0 + 16,
	1 + 16,
	4 + 16,
	6 + 16,
	3 + 16
};

#define SPECASM_128_PARSE_BANK 0
#define SPECASM_128_CLIP_BANK 1
#define SPECASM_128_DUMP_BANK 2
#define SPECASM_128_EDITOR_BANK 3
#define SPECASM_128_HELP_BANK 4

#ifdef UNITTESTS
#define SPECASM_128_UNIT_BANK 3
#endif


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
void specasm_help_banked(const char *ins_name);

#ifdef UNITTESTS
void specasm_peer_write_state_banked_e(const char *fname, uint16_t checksum);
uint16_t specasm_peer_read_state_banked_e(const char *fname);
#endif

void specasm_trampolines_init(void)
{
	uint8_t* spectrum_type = (uint8_t*) 2899;

	if (*spectrum_type != 126)
		return;

	banks[SPECASM_128_PARSE_BANK] = 0 + 16;
	banks[SPECASM_128_CLIP_BANK] = 4 + 16;
	banks[SPECASM_128_DUMP_BANK] = 1 + 16;
	banks[SPECASM_128_EDITOR_BANK] = 3 + 16;
	banks[SPECASM_128_HELP_BANK] = 6 + 16;
}

/* clang-format off */

/*
 * Interrupts are disabled for Specasm globally.  We don't want to disable and
 * enable here.
 */

static uint8_t prv_map_bank(uint8_t bank) __z88dk_fastcall __naked __z88dk_callee
{
__asm
	push af
	push bc
	ld a, (0x5b5c)
	push af
	and 0xe0
	or l
	ld bc, 0x7ffd
	ld (0x5b5c), a
	out (c), a
	pop af
	and 0x1f
	ld l, a
	pop bc
	pop af
	ret
__endasm;
}

/* clang-format on */


char *specasm_get_long_imm_e(const char *str, long *val, uint8_t *flags)
{
	uint8_t bank;
	char *e;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	e = specasm_get_long_imm_banked_e(str, val, flags);
	(void) prv_map_bank(bank);

	return e;
}

void specasm_append_empty_line_e(void)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	specasm_append_empty_line_banked_e();
	(void) prv_map_bank(bank);
}

void specasm_delete_lines(unsigned int start, unsigned int end)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	specasm_delete_lines_banked(start, end);
	(void) prv_map_bank(bank);
}

void specasm_insert_lines_e(unsigned int l, unsigned int count)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	specasm_insert_lines_banked_e(l, count);
	(void) prv_map_bank(bank);
}

void specasm_parse_line_e(unsigned int l, const char *str)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	specasm_parse_line_banked_e(l, str);
	(void) prv_map_bank(bank);
}

/** EXPORT Needed by tests**/
uint8_t specasm_parse_mnemomic_e(const char *str, uint8_t i,
				 specasm_line_t *line)
{
	uint8_t e;
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_PARSE_BANK]);
	e = specasm_parse_mnemomic_banked_e(str, i, line);
	(void) prv_map_bank(bank);

	return e;
}

/** EXPORT Needed by tests**/
void specasm_init_dump_table(void)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_DUMP_BANK]);
	specasm_init_dump_table_banked();
	(void) prv_map_bank(bank);
}

/** EXPORT Needed by tests**/
uint8_t specasm_dump_opcode_e(const specasm_line_t *line, char *buf)
{
	uint8_t e;
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_DUMP_BANK]);
	e = specasm_dump_opcode_banked_e(line, buf);
	(void) prv_map_bank(bank);

	return e;
}

/* EXPORT */
void specasm_format_line_e(char *buf, unsigned int l)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_DUMP_BANK]);
	specasm_format_line_banked_e(buf, l);
	(void) prv_map_bank(bank);
}

void specasm_clip_reset(void)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_CLIP_BANK]);
	specasm_clip_reset_banked();
	(void) prv_map_bank(bank);
}

void specasm_clip_add_line_e(const char *line)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_CLIP_BANK]);
	specasm_clip_add_line_banked_e(line);
	(void) prv_map_bank(bank);
}

uint16_t specasm_clip_get_line(uint16_t ptr, char *buffer)
{
	uint16_t e;
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_CLIP_BANK]);
	e = specasm_clip_get_line_banked(ptr, buffer);
	(void) prv_map_bank(bank);

	return e;
}

uint16_t specasm_clip_get_line_count(void)
{
	uint16_t e;
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_CLIP_BANK]);
	e = specasm_clip_get_line_count_banked();
	(void) prv_map_bank(bank);

	return e;
}

#ifndef UNITTESTS
void specasm_draw_status(void)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_EDITOR_BANK]);
	specasm_draw_status_banked();
	(void) prv_map_bank(bank);
}

void specasm_handle_key_press(uint8_t k)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_EDITOR_BANK]);
	specasm_handle_key_press_banked(k);
	(void) prv_map_bank(bank);
}

void specasm_editor_reset(void)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_EDITOR_BANK]);
	specasm_editor_reset_banked();
	(void) prv_map_bank(bank);
}

void specasm_help(const char *ins_name)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_HELP_BANK]);
	specasm_help_banked(ins_name);
	(void) prv_map_bank(bank);
}
#else
void specasm_peer_write_state_e(const char *fname, uint16_t checksum)
{
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_UNIT_BANK]);
	specasm_peer_write_state_banked_e(fname, checksum);
	(void) prv_map_bank(bank);
}

uint16_t specasm_peer_read_state_e(const char *fname)
{
	uint16_t e;
	uint8_t bank;

	bank = prv_map_bank(banks[SPECASM_128_UNIT_BANK]);
	e = specasm_peer_read_state_banked_e(fname);
	(void) prv_map_bank(bank);

	return e;
}
#endif
