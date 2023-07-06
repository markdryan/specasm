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

#include <string.h>

#include "error.h"
#include "sbc_error.h"
#include "sbc_parser.h"
#include "sbc_overlay.h"

static void prv_dump_e(const char *fname)
{

	sbc_parse_file_e(fname);
	if (err_type != SPECASM_ERROR_OK)
		return;

	printf("\n");
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: sbcfmt .sbc\n");
		return 1;
	}

	prv_dump_e(argv[1]);

	if ((err_type == SBC_ERROR_OPEN) || (err_type == SBC_ERROR_WRITE)) {
		printf("%s : %s\n", sbc_error_msg(), argv[1]);
		return 1;
	}

	if (err_type != SPECASM_ERROR_OK) {
		printf("%s at line %d\n", sbc_error_msg(), overlay.lex.line_no);
		return 1;
	}

	return 0;
}

