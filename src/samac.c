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

#include "error.h"
#include "scratch.h"
#include "state.h"

#include <string.h>
static char samac_scratch[SPECASM_MAX_SCRATCH + 1];

extern void specasm_samac_init(void);
void specasm_samac_new(void) { specasm_state_reset(); }

/*
 * TODO: Copied from editor.c.  Banking makes sharing of the function trickier,
 * so it's better to factorize this code in it's own commit.  Best not to
 * duplicate it long term in case we add more file types.  Note the code
 * is a bit different as in the editor we subtract one from the length to
 * get rid of the l or s and also I think the length is 1 less than it should
 * be, so refactor with caution.
 */

static char *prv_complete_filename_e(char *com, uint8_t len)
{
	char *com2;

	while (*com == ' ') {
		++com;
		len--;
	}

	/*
	 * Do we already have a valid extension?
	 */

	com2 = com + (len - 1);
	if ((len >= 3) && ((*com2 == 't') || (*com2 == 'x'))) {
		com2--;
		if (*com2 == '.')
			return com;
	}

	while ((com2 > com) && (*com2 != '.') && (*com2 != '/'))
		com2--;

	/*
	 * We've got another extension where there shouldn't be one.
	 */

	if (*com2 == '.') {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return NULL;
	}

	/*
	 * If we get here the last part of the file name does not contain an
	 * extension.  There must be an extension so we'll add the default
	 * one before loading, '.x'.
	 */

	if (len + 2 >= SPECASM_MAX_SCRATCH) {
		err_type = SPECASM_ERROR_BAD_FNAME;
		return NULL;
	}
	com2 = com + len;
	com2[0] = '.';
	com2[1] = 'x';
	com2[2] = 0;

	return com;
}

specasm_error_t specasm_samac_asm(const char *str, uint16_t len)
{
	uint8_t i;
	specasm_error_t err;
	char *ptr;
	uint16_t cur_line = state.lines.num_lines;

	if (len > SPECASM_LINE_MAX_LEN)
		return SPECASM_ERROR_NO_ROOM_IN_LINE;

	specasm_append_empty_line_e();
	if ((err_type != SPECASM_ERROR_OK) || (len == 0))
		return err_type;

	memset(samac_scratch, ' ', SPECASM_LINE_MAX_LEN);

	/*
	 * Similar to saimport, we'll ignore characters < ' '
	 */
	ptr = samac_scratch;
	for (i = 0; i < (uint8_t)len; i++) {
		if (str[i] >= 32)
			*ptr++ = str[i];
	}
	samac_scratch[SPECASM_LINE_MAX_LEN] = 0;

	specasm_parse_line_e(cur_line, samac_scratch);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_format_line_e(samac_scratch, cur_line);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	return SPECASM_ERROR_OK;

on_error:
	err = err_type;
	specasm_delete_lines(cur_line, cur_line + 1);
	err_type = SPECASM_ERROR_OK;
	return err;
}

specasm_error_t specasm_samac_load(const char *str, uint16_t len)
{
	const char *fname;
	specasm_error_t err;

	if (len >= SPECASM_MAX_SCRATCH)
		return SPECASM_ERROR_BAD_FNAME;

	memcpy(samac_scratch, str, len);
	samac_scratch[len] = 0;

	fname = prv_complete_filename_e(samac_scratch, len);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_load_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	return SPECASM_ERROR_OK;

on_error:
	err = err_type;
	if (err_type != SPECASM_ERROR_OPEN)
		specasm_state_reset();
	err_type = SPECASM_ERROR_OK;
	return err;
}

specasm_error_t specasm_samac_save(const char *str, uint16_t len)
{
	const char *fname;
	specasm_error_t err;

	if (len >= SPECASM_MAX_SCRATCH)
		return SPECASM_ERROR_BAD_FNAME;

	memcpy(samac_scratch, str, len);
	samac_scratch[len] = 0;

	fname = prv_complete_filename_e(samac_scratch, len);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	specasm_save_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		goto on_error;

	return SPECASM_ERROR_OK;

on_error:
	err = err_type;
	err_type = SPECASM_ERROR_OK;
	return err;
}

int main(int argc, char *argv[])
{
	specasm_init_dump_table();
	specasm_state_reset();
	specasm_samac_init();
	return 0;
}
