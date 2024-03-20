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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "peer.h"
#include "salink.h"
#include "state_base.h"

#define SAMAKE_TARGET_TYPE_NONE 0
#define SAMAKE_TARGET_TYPE_BAS 1
#define SAMAKE_TARGET_TYPE_TAP 2
#define SAMAKE_TARGET_TYPE_P 3

#define SAMAKE_CODE_BUF_SIZE 1024

static char bin_name[MAX_FNAME + 1];
static char app_name[MAX_FNAME + 1];
static uint8_t bin_name_len;
static char start_address[6] = "32768";
static char clear_address[6] = "32767";
static uint16_t org_address = 0x8000;
static uint16_t basic_prog_len;
static uint8_t got_org;
static uint8_t got_zx81;

#define SAMAKE_ERROR_NO_MAIN SPECASM_MAX_ERRORS
#define SAMAKE_ERROR_USAGE (SPECASM_MAX_ERRORS + 1)
#define SAMAKE_ERROR_READDIR (SPECASM_MAX_ERRORS + 2)
#define SAMAKE_ERROR_BIN_TOO_BIG (SPECASM_MAX_ERRORS + 3)
#define SAMAKE_ERROR_BAD_ORG (SPECASM_MAX_ERRORS + 4)
#define SAMAKE_ERROR_BAD_TYPE (SPECASM_MAX_ERRORS + 5)

/*
 * Buffer for the Spectrum, Next and ZX81 BASIC loaders.  This is
 * going to be 42 + MAX_FNAME bytes for the Spectrum and the Next.
 * For the ZX81 it's going to be 121 bytes.  We'll round up to
 * 128 bytes.
 */

static uint8_t basic_buf[128];
static union {
	uint8_t three_dos_buf[128];
	uint8_t tap_block[24];
	uint8_t code_buf[SAMAKE_CODE_BUF_SIZE];
} container;

static specasm_dirent_t dirent;

static uint8_t prv_parse_obj_e(const char *fname)
{
	uint16_t i;
	specasm_line_t *line;
	const char *str;
	uint16_t sa;
	char *dst;

	specasm_load_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	for (i = 0; i < state.lines.num_lines; i++) {
		line = &state.lines.lines[i];
		if (!bin_name[0] && ((line->type == SPECASM_LINE_TYPE_LL) ||
				     (line->type == SPECASM_LINE_TYPE_SL))) {
			if (line->type == SPECASM_LINE_TYPE_LL)
				str =
				    specasm_state_get_long_e(line->data.label);
			else
				str =
				    specasm_state_get_short_e(line->data.label);
			if (err_type != SPECASM_ERROR_OK)
				return 0;
			if (strcmp(str, "Main"))
				continue;
			dst = &bin_name[0];
			while (*fname != '.')
				*dst++ = *fname++;
			*dst = 0;
			bin_name_len = dst - &bin_name[0];
			if (got_org && got_zx81)
				return 1;
		} else if (!got_org && (line->type == SPECASM_LINE_TYPE_ORG)) {
			sa = *((uint16_t *)&line->data.op_code[0]);
			org_address = sa;
			got_org = 1;
			(void)utoa(sa, start_address, 10);
			sa--;
			(void)utoa(sa, clear_address, 10);
			if (bin_name[0] && got_zx81)
				return 1;
		} else if (!got_zx81 &&
			   (line->type == SPECASM_LINE_TYPE_ZX81)) {
			got_zx81 = 1;
			if (bin_name[0] && got_org)
				return 1;
		}
	}

	return 0;
}

static uint8_t prv_check_file(const char *fname)
{
	char *period;

	period = strchr(fname, '.');

	if (!period)
		return 0;

	return ((period[1] == 'x' || period[1] == 'X') && period[2] == 0);
}

static void prv_find_bin_name_e(const char *dirname, uint8_t target_type)
{
	uint8_t done = 0;
	specasm_dir_t dir = specasm_opendir_e(dirname);

	if (err_type != SPECASM_ERROR_OK) {
		printf("Failed to read directory: %s", dirname);
		err_type = SAMAKE_ERROR_READDIR;
		return;
	}

	while (!done && specasm_readdir(dir, &dirent)) {
		if (!prv_check_file(specasm_getdirname(dirent)))
			continue;

		done = prv_parse_obj_e(specasm_getdirname(dirent));
		if (err_type != SPECASM_ERROR_OK)
			goto finish;
	}

	if (!bin_name[0]) {
		err_type = SAMAKE_ERROR_NO_MAIN;
		printf("Unable to find 'Main' label in %s\n", dirname);
	}

finish:
	specasm_closedir(dir);
}

static void prv_make_app_name(const char *type)
{
	uint8_t ext_pos;
	uint8_t bin_name_len_plus_ext;

	bin_name_len_plus_ext = bin_name_len + strlen(type) + 1;
	if (bin_name_len_plus_ext > MAX_FNAME)
		ext_pos = bin_name_len - (bin_name_len_plus_ext - MAX_FNAME);
	else
		ext_pos = bin_name_len;
	strcpy(app_name, bin_name);
	app_name[ext_pos] = '.';
	strcpy(&app_name[ext_pos + 1], type);
}

static uint8_t *prv_write_address(uint8_t *ptr, const char *address)
{
	*ptr++ = 0xb0; /* VAL */
	*ptr++ = '"';
	memcpy(ptr, address, 5);
	ptr += 5;
	*ptr++ = '"';

	return ptr;
}

static void prv_make_basic_file(uint8_t star, const char *code_name,
				uint8_t code_name_len)
{
	uint16_t line_len;
	uint8_t *ptr;

	basic_buf[1] = 0xa;
	basic_buf[4] = 0xfd; /* CLEAR */
	(void)prv_write_address(&basic_buf[5], clear_address);
	basic_buf[13] = ':';
	basic_buf[14] = 0xef; /* LOAD */
	ptr = &basic_buf[15];
	if (star)
		*ptr++ = '*';
	*ptr++ = '"';
	if (code_name_len > 0) {
		memcpy(ptr, code_name, code_name_len);
		ptr += code_name_len;
	}
	*ptr++ = '"';
	*ptr++ = 0xaf; /* CODE */
	if (code_name_len > 0)
		ptr = prv_write_address(ptr, start_address);
	*ptr++ = ':';
	*ptr++ = 0xf9; /* RANDOMIZE */
	*ptr++ = 0xc0; /* USR */
	ptr = prv_write_address(ptr, start_address);
	*ptr++ = 0x0d;

	/*
	 * Write line length.
	 */

	line_len = ptr - &basic_buf[4];
	memcpy(&basic_buf[2], &line_len, sizeof(uint16_t));
	basic_prog_len = (uint16_t)(ptr - basic_buf);
}

static void prv_make_3dos_header(void)
{
	uint8_t i;
	uint32_t file_len = basic_prog_len + 128;
	uint8_t checksum = 0;

	memcpy(container.three_dos_buf, "PLUS3DOS", 8);
	container.three_dos_buf[8] = 0x1a;
	container.three_dos_buf[9] = 1;
	memcpy(&container.three_dos_buf[11], &file_len, sizeof(uint32_t));
	memcpy(&container.three_dos_buf[16], &basic_prog_len, sizeof(uint16_t));
	container.three_dos_buf[18] = 10; /* LINE 10 */
	memcpy(&container.three_dos_buf[20], &basic_prog_len, sizeof(uint16_t));

	for (i = 0; i < 127; i++)
		checksum += container.three_dos_buf[i];
	container.three_dos_buf[127] = checksum;
}

static specasm_handle_t prv_open_bin_e(uint16_t *bin_size)
{
	specasm_handle_t in_f;
	uint32_t real_bin_size;
	specasm_stat_t stat_buf;

	in_f = specasm_file_ropen_e(bin_name);
	if (err_type != SPECASM_ERROR_OK)
		return 0;

	specasm_file_stat_e(in_f, &stat_buf);
	if (err_type != SPECASM_ERROR_OK)
		goto close_in_f;

	real_bin_size = specasm_get_file_size(&stat_buf);

	if (real_bin_size > 0xffff) {
		err_type = SAMAKE_ERROR_BIN_TOO_BIG;
		printf("%s too big (%" PRIu32 " bytes)\n", bin_name,
		       real_bin_size);
		goto close_in_f;
	}

	if (real_bin_size + (uint32_t)org_address > 0xffff) {
		err_type = SAMAKE_ERROR_BIN_TOO_BIG;
		printf("%s too big (%" PRIu32 " bytes) for org %" PRIu16 "\n",
		       bin_name, real_bin_size, org_address);
		goto close_in_f;
	}

	*bin_size = (uint16_t)real_bin_size;

	return in_f;

close_in_f:
	specasm_file_close_e(in_f);

	return 0;
}

static void prv_make_bas_e(void)
{
	specasm_handle_t f;
	uint16_t bin_size;

	/* Check bin file exists and is not too big. */

	f = prv_open_bin_e(&bin_size);
	if (err_type != SPECASM_ERROR_OK)
		return;
	specasm_file_close_e(f);
	err_type = SPECASM_ERROR_OK;

	prv_make_app_name("bas");
#ifdef SPECASM_TARGET_NEXT
	prv_make_basic_file(0, bin_name, bin_name_len);
#else
	prv_make_basic_file(1, bin_name, bin_name_len);
#endif
	prv_make_3dos_header();

	f = specasm_file_wopen_e(app_name);
	if (err_type != SPECASM_ERROR_OK)
		return;

	specasm_file_write_e(f, container.three_dos_buf, 128);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_write_e(f, basic_buf, basic_prog_len);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_close_e(f);
	return;

on_error:
	specasm_file_close_e(f);
	specasm_remove_file(app_name);
}

static void prv_make_basic_header(void)
{
	uint8_t i;
	uint16_t block_len = basic_prog_len + 2;
	uint8_t name_len = bin_name_len;

	if (name_len > 10)
		name_len = 10;

	container.tap_block[0] = 0x13; /* Header len. */
	container.tap_block[2] = 0;    /* Flag byte, 0 = header */
	container.tap_block[3] = 0;    /* Type byte, 0 = program */

	/* Copy the name of the BASIC file, we'll just use the bin file */

	memset(&container.tap_block[4], ' ', 10);
	memcpy(&container.tap_block[4], bin_name, name_len);
	memcpy(&container.tap_block[14], &basic_prog_len,
	       sizeof(basic_prog_len));

	container.tap_block[16] = 10; /* Autostart at line 10 */
	memcpy(&container.tap_block[18], &basic_prog_len,
	       sizeof(basic_prog_len));
	container.tap_block[20] = 0;

	for (i = 2; i < 20; i++)
		container.tap_block[20] ^= container.tap_block[i];

	/*
	 * Set len of second block.
	 */

	memcpy(&container.tap_block[21], &block_len, sizeof(block_len));
	container.tap_block[23] = 0xff;
}

static void prv_make_code_header(uint16_t bin_size)
{
	uint8_t i;
	uint16_t block_len = bin_size + 2;
	uint8_t name_len = bin_name_len;

	if (name_len > 10)
		name_len = 10;

	container.tap_block[0] = 0x13; /* Header len. */
	container.tap_block[1] = 0;
	container.tap_block[2] = 0; /* Flag byte, 0 = header */
	container.tap_block[3] = 3; /* Type byte, 0 = program */

	/* Copy the name of the BASIC file, we'll just use the bin file */

	memset(&container.tap_block[4], ' ', 10);
	memcpy(&container.tap_block[4], bin_name, name_len);
	memcpy(&container.tap_block[14], &bin_size, sizeof(bin_size));

	/* Start of Code block */
	memcpy(&container.tap_block[16], &org_address, sizeof(org_address));

	/* 32768 */
	container.tap_block[18] = 0;
	container.tap_block[19] = 0x80;
	container.tap_block[20] = 0;

	for (i = 2; i < 20; i++)
		container.tap_block[20] ^= container.tap_block[i];

	/*
	 * Set len of second block.
	 */

	memcpy(&container.tap_block[21], &block_len, sizeof(block_len));
	container.tap_block[23] = 0xff;
}

static uint8_t prv_write_code_e(specasm_handle_t in_f, specasm_handle_t out_f)
{
	uint16_t i;
	uint8_t checksum = 0xff;
	size_t read;

	for (;;) {
		read = specasm_file_read_e(in_f, &container.code_buf[0],
					   SAMAKE_CODE_BUF_SIZE);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
		if (!read)
			break;
		for (i = 0; i < read; i++)
			checksum ^= container.code_buf[i];
		specasm_file_write_e(out_f, &container.code_buf[0], read);
		if (err_type != SPECASM_ERROR_OK)
			return 0;
	}

	return checksum;
}

static void prv_make_tap_e(void)
{
	specasm_handle_t out_f;
	specasm_handle_t in_f;
	uint16_t bin_size;
	uint16_t i;
	uint8_t checksum = 0xff;

	prv_make_app_name("tap");
	prv_make_basic_file(0, "", 0);

	prv_make_basic_header();

	in_f = prv_open_bin_e(&bin_size);
	if (err_type != SPECASM_ERROR_OK)
		return;

	/*
	 * That's the header.  Now we can write our BASIC program followed
	 * by the checksum.
	 */

	out_f = specasm_file_wopen_e(app_name);
	if (err_type != SPECASM_ERROR_OK) {
		specasm_file_close_e(in_f);
		return;
	}

	specasm_file_write_e(out_f, &container.tap_block[0], 24);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_write_e(out_f, basic_buf, basic_prog_len);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	for (i = 0; i < basic_prog_len; i++)
		checksum ^= basic_buf[i];

	specasm_file_write_e(out_f, &checksum, 1);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	/*
	 * Let's write the header for the code block.
	 */

	prv_make_code_header(bin_size);

	specasm_file_write_e(out_f, &container.tap_block[0], 24);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	/*
	 * Now write out code file and compute the checksum.
	 */

	checksum = prv_write_code_e(in_f, out_f);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_write_e(out_f, &checksum, 1);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_close_e(in_f);
	specasm_file_close_e(out_f);
	return;

on_error:
	specasm_file_close_e(in_f);
	specasm_file_close_e(out_f);
	specasm_remove_file(app_name);
}

static void prv_make_p_header(uint16_t len)
{
	uint16_t d_file;
	uint16_t df_cc;
	uint16_t vars;
	uint16_t e_line;
	uint16_t ch_add;
	uint16_t stkbot;
	uint16_t len_plus_2;

	d_file = 16530u + len;
	df_cc = d_file + 1;
	vars = d_file + 793;
	e_line = vars + 1;
	ch_add = e_line + 4;
	stkbot = ch_add + 1;

	basic_buf[1] = 1;                   /* E_PPC */
	basic_buf[3] = (d_file & 0xFF);     /* D_FILE LO */
	basic_buf[4] = (d_file >> 8);       /* D_FILE HI */
	basic_buf[5] = (df_cc & 0xFF);      /* D_FCC LO */
	basic_buf[6] = (df_cc >> 8);        /* D_FCC HI */
	basic_buf[7] = (vars & 0xFF);       /* VARS LO */
	basic_buf[8] = (vars >> 8);         /* VARS HI */
	/* basic_buf[9] and basic_buf[10] is   DEST */
	basic_buf[11] = (e_line & 0xFF);    /* E_LINE LO */
	basic_buf[12] = (e_line >> 8);      /* E_LINE HI */
	basic_buf[13] = (ch_add & 0xFF);    /* CH_ADD LO */
	basic_buf[14] = (ch_add >> 8);      /* CH_ADD HI */
	/* basic_buf[15] and basic_buf[16] is  X_PTR */
	basic_buf[17] = (stkbot & 0xFF);    /* STKBOT LO */
	basic_buf[18] = (stkbot >> 8);      /* STKBOT HI */
	basic_buf[19] = (stkbot & 0xFF);    /* STKEND LO */
	basic_buf[20] = (stkbot >> 8);      /* STKEND HI */
	/* basic_buf[21] is                    BERG */
	basic_buf[22] = (16477u & 0xFF);    /* MEM LO */
	basic_buf[23] = (16477u >> 8);      /* MEM HI */
	/* basic_buf[24] is not used */
	basic_buf[25] = 2;                  /* DF_SZ */
	/* basic_buf[26] and basic_buf[27] is  S_TOP */
	basic_buf[28] = (64959u & 0xff);    /* LAST_K */
	basic_buf[29] = (64959u >> 8);      /* LAST_K */
	basic_buf[30] = 255;                /* Debouce */
	basic_buf[31] = 55;                 /* MARGIN */
	basic_buf[32] = (16509u & 0xff);   /* NXTLIN */
	basic_buf[33] = (16509u >> 8);      /* NXTLIN */
	/* basic_buf[34] and basic_buf[35] is  OLDPPC */
	/* basic_buf[36] is                    FLAGX */
	/* basic_buf[37] and basic_buf[38] is  STRLEN */
	basic_buf[39] = 0x8d;               /* T_ADDR */
	basic_buf[40] = 0xc;
	/* basic_buf[41] and basic_buf[42] is  SEED */
	basic_buf[43] = (63000u & 0xff);   /* FRAMES */
	basic_buf[44] = (63000u >> 8);      /* FRAMES */
	/* basic_buf[45] and basic_buf[46] is  COORDS */
	basic_buf[47] = 188;                /* PR_CC */
	basic_buf[48] = 33;                 /* S_POSN (X) */
	basic_buf[49] = 24;                 /* S_POSN (Y) */
	basic_buf[50] = 64;                 /* CDFLAG */
	/* basic_buf[51] .. basic_buf[82]      PRBUFF */
	basic_buf[83] = 118;
	/* basic_buf[84] .. basic_buf[114]      MEMBOT */
	/* basic_buf[114] and basic_buf[115] is SPARE */

	/*
	 * Write the start of the BASIC program.
	 */

	basic_buf[116] = 0;                /*  Line No */
	basic_buf[117] = 1;

	len_plus_2 = len + 2;
	basic_buf[118] = len_plus_2 & 0xff;
	basic_buf[119] = len_plus_2 >> 8;
	basic_buf[120] = 234;
}

static void prv_make_p_e(void)
{
	specasm_handle_t out_f;
	specasm_handle_t in_f;
	uint16_t bin_size;
	uint16_t i;

	const uint8_t loader[] = {
		118, 0, 2, 11, 0, 249, 212, 197, 11, 29,
		34, 33, 29, 32, 11, 118
	};
	const uint8_t footer[] = { 118, 128 };

	if (got_org && strcmp(start_address, "16514")) {
		printf("Bad ORG address %s, want 16514", start_address);
		err_type = SAMAKE_ERROR_BAD_ORG;
		return;
	}

	prv_make_app_name("p");

	in_f = prv_open_bin_e(&bin_size);
	if (err_type != SPECASM_ERROR_OK)
		return;

	prv_make_p_header(bin_size);

	out_f = specasm_file_wopen_e(app_name);
	if (err_type != SPECASM_ERROR_OK) {
		specasm_file_close_e(in_f);
		return;
	}

	specasm_file_write_e(out_f, &basic_buf[0], 121);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	(void) prv_write_code_e(in_f, out_f);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_write_e(out_f, &loader[0], sizeof(loader));
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	/*
	 * Now write empty display file.
	 */

	memset(&basic_buf[1], 0, 32);
	basic_buf[0] = 118;

	for (i = 0; i < 24; i++) {
		specasm_file_write_e(out_f, &basic_buf[0], 33);
		if (err_type != SPECASM_ERROR_OK)
			goto on_error;
	}

	specasm_file_write_e(out_f, &footer[0], sizeof(footer));
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_file_close_e(in_f);
	specasm_file_close_e(out_f);
	return;

on_error:
	specasm_file_close_e(in_f);
	specasm_file_close_e(out_f);
	specasm_remove_file(app_name);

}

static void prv_make_e(const char *dir, uint8_t target_type)
{
	prv_find_bin_name_e(dir, target_type);
	if (err_type != SPECASM_ERROR_OK)
		return;

	if (got_zx81 && (target_type != SAMAKE_TARGET_TYPE_P) &&
	    (target_type != SAMAKE_TARGET_TYPE_NONE)) {
		printf("p is the only type supported for the zx81\n");
		err_type = SAMAKE_ERROR_BAD_TYPE;
		return;
	}

	/*
	 * We could perform the opposite check here, that if you select p
	 * and your program doesn't include the zx81 directive then we report
	 * an error.  The thing is though that this should still work, providing
	 * you set the org correctly and do the character conversion yourself.
	 * This might actually be something you want to do if you have some
	 * existing code that does the conversion at runtime.
	 */

	if (target_type == SAMAKE_TARGET_TYPE_NONE)
		target_type = got_zx81 ? SAMAKE_TARGET_TYPE_P :
			SAMAKE_TARGET_TYPE_BAS;

	/*
	 * TODO, this check isn't really correct.  Ideally we'd add the loading
	 * address of BASIC program and the size of the BASIC program and check
	 * that the resulting value isn't greater than org_address, but I can't
	 * figure out whether there's a fixed starting address for BASIC
	 * programs, so for now let's just print a warning.  It's mainly there
	 * to stop people creating a loader for a dot program.
	 */

	if ((target_type != SAMAKE_TARGET_TYPE_P) && (org_address < 24000))
		printf("Warning: org %" PRIu16 " is very low\n", org_address);

	if (target_type == SAMAKE_TARGET_TYPE_BAS)
		prv_make_bas_e();
	else if (target_type == SAMAKE_TARGET_TYPE_TAP)
		prv_make_tap_e();
	else
		prv_make_p_e();
}

int main(int argc, char *argv[])
{
	const char *dir = ".";
	uint8_t target_type = SAMAKE_TARGET_TYPE_NONE;
	int ret = 0;

#ifdef SPECASM_TARGET_NEXT
	uint8_t turbo;

	/*
	 * Create loader at top speed on a spectrum Next.
	 */

	turbo = ZXN_READ_REG(REG_TURBO_MODE);
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo | 3);
#endif

	if (argc >= 2) {
		if (!strcmp(argv[1], "tap")) {
			target_type = SAMAKE_TARGET_TYPE_TAP;
		} else if (!strcmp(argv[1], "p")) {
			target_type = SAMAKE_TARGET_TYPE_P;
		} else if (!strcmp(argv[1], "bas")) {
			target_type = SAMAKE_TARGET_TYPE_BAS;
		} else {
			err_type = SAMAKE_ERROR_USAGE;
			goto on_error;
		}

		if (argc == 3) {
			dir = argv[2];
		} else if (argc > 3) {
			err_type = SAMAKE_ERROR_USAGE;
			goto on_error;
		}
	}
	prv_make_e(dir, target_type);

on_error:
	if (err_type != SPECASM_ERROR_OK) {
		if (err_type < SPECASM_MAX_ERRORS)
			printf("%s\n", specasm_error_msg(err_type));
		else if (err_type == SAMAKE_ERROR_USAGE)
			printf("Usage: samake (bas|tap) [dir]\n");
		ret = 1;
	} else {
		printf("Created %s\n", app_name);
	}

#ifdef SPECASM_TARGET_NEXT
	ZXN_WRITE_REG(REG_TURBO_MODE, turbo);
#endif
	return ret;
}
