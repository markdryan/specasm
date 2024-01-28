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

#include "line_common.h"
#include "peer.h"
#include "state.h"

/*
 * This file contains some functions from line_dump.c that are too big to
 * fit into an 8KB page.  They're stored in this file purely so they can be
 * stored in the main segment on the Next.
 */

uint8_t specasm_dump_byte(char *buf, uint8_t v, uint8_t flags)
{
	int iv;
	unsigned char radix;
	const char *start;

	if (flags == SPECASM_FLAGS_NUM_CHAR) {
		buf[0] = '\'';
		buf[1] = v;
		buf[2] = '\'';
		buf[3] = 0;
		return 3;
	}

	start = buf;
	if (flags == SPECASM_FLAGS_NUM_HEX) {
		*buf++ = '$';
		radix = 16;
		iv = v;
	} else {
		radix = 10;
		if (flags == SPECASM_FLAGS_NUM_SIGNED)
			iv = (int8_t)v;
		else
			iv = v;
	}
	(void)itoa(iv, buf, radix);
	return strlen(start);
}

uint8_t specasm_dump_word(char *buf, uint16_t v, uint8_t flags)
{
	const char *start;
	unsigned char radix;

	if (flags == SPECASM_FLAGS_NUM_CHAR) {
		buf[0] = '\'';
		buf[1] = (uint8_t)v;
		buf[2] = '\'';
		buf[3] = 0;
		return 3;
	}

	start = buf;
	if (flags == SPECASM_FLAGS_NUM_SIGNED) {
		(void)itoa(v, buf, 10);
	} else {
		if (flags == SPECASM_FLAGS_NUM_HEX) {
			*buf++ = '$';
			radix = 16;
		} else {
			radix = 10;
		}

		(void)utoa(v, buf, radix);
	}

	return strlen(start);
}

char *specasm_dump_index(const uint8_t *op_code, char *buf, uint8_t flags)
{
	buf[0] = '(';
	buf[1] = 'i';
	buf[2] = op_code[0] == 0xDD ? 'x' : 'y';
	buf[3] = '+';
	buf += 4;
	buf += specasm_dump_byte(buf, op_code[2], flags);
	*buf++ = ')';
	return buf;
}
