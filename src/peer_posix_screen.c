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
#include <stdio.h>
#include <string.h>

#include "peer.h"

void specasm_screen_flush(uint16_t peer_last_row)
{
	char line[SPECASM_LINE_MAX_LEN + 1];

	line[SPECASM_LINE_MAX_LEN] = 0;
	for (size_t i = 0; i <= peer_last_row; i++) {
		size_t offset = (i * SPECASM_LINE_MAX_LEN);
		memcpy(line, &peer_unit_screen[offset], SPECASM_LINE_MAX_LEN);
		printf("%s\n", line);
	}
}

int itoa(int n, char *s, unsigned char radix)
{
	return sprintf(s, radix == 16 ? "%" PRIX16 : "%" PRId16, (int16_t)n);
}
int utoa(int n, char *s, unsigned char radix)
{
	return sprintf(s, radix == 16 ? "%" PRIX16 : "%" PRIu16, (uint16_t)n);
}
