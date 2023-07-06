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

#ifndef SBC_LEXER_H
#define SBC_LEXER_H

#include <stdint.h>

#include "peer_file.h"
#include "sbc_config.h"

#define SBC_LEX_BUF_SIZE 1024

#define SBC_TOKEN_EOF 0
#define SBC_TOKEN_LINE_LABEL 1
#define SBC_TOKEN_KEYWORD 2
#define SBC_TOKEN_OPERATOR 3
#define SBC_TOKEN_STRING 4
#define SBC_TOKEN_INTEGER 5
#define SBC_TOKEN_HEX 6
#define SBC_TOKEN_BIN 7
#define SBC_TOKEN_REAL 8
#define SBC_TOKEN_IDENTIFIER 9
#define SBC_TOKEN_LINE_NUMBER 10
#define SBC_TOKEN_REM 11
#define SBC_TOKEN_UNKNOWN 12

typedef uint8_t sbc_token_type_t;

#define SBC_ID_TYPE_NONE 0
#define SBC_ID_TYPE_INT 1
#define SBC_ID_TYPE_REAL 2
#define SBC_ID_TYPE_STR 3

typedef uint8_t sbc_id_type_t;

#define SBC_KEYWORD_OTHERWISE 0
#define SBC_KEYWORD_AND 1
#define SBC_KEYWORD_DIV 2
#define SBC_KEYWORD_EOR 3
#define SBC_KEYWORD_MOD 4
#define SBC_KEYWORD_OR 5
#define SBC_KEYWORD_ERROR 6
#define SBC_KEYWORD_LINE 7
#define SBC_KEYWORD_OFF 8
#define SBC_KEYWORD_STEP 9
#define SBC_KEYWORD_SPC 10
#define SBC_KEYWORD_TAB 11
#define SBC_KEYWORD_ELSE 12
#define SBC_KEYWORD_THEN 13

#define SBC_KEYWORD_OPENIN 15
#define SBC_KEYWORD_PTR 16
#define SBC_KEYWORD_PAGE 17
#define SBC_KEYWORD_TIME 18
#define SBC_KEYWORD_LOMEM 19
#define SBC_KEYWORD_HIMEM 20
#define SBC_KEYWORD_ABS 21
#define SBC_KEYWORD_ACS 22
#define SBC_KEYWORD_ADVAL 23
#define SBC_KEYWORD_ASC 24
#define SBC_KEYWORD_ASN 25
#define SBC_KEYWORD_ATN 26
#define SBC_KEYWORD_BGET 27
#define SBC_KEYWORD_COS 28
#define SBC_KEYWORD_COUNT 29
#define SBC_KEYWORD_DEG 30
#define SBC_KEYWORD_ERL 31
#define SBC_KEYWORD_ERR 32
#define SBC_KEYWORD_EVAL 33
#define SBC_KEYWORD_EXP 34
#define SBC_KEYWORD_EXT 35
#define SBC_KEYWORD_FALSE 36
#define SBC_KEYWORD_FN 37
#define SBC_KEYWORD_GET 38
#define SBC_KEYWORD_INKEY 39
#define SBC_KEYWORD_INSTR 40
#define SBC_KEYWORD_INT 41
#define SBC_KEYWORD_LEN 42
#define SBC_KEYWORD_LN 43
#define SBC_KEYWORD_LOG 44
#define SBC_KEYWORD_NOT 45
#define SBC_KEYWORD_OPENUP 46
#define SBC_KEYWORD_OPENOUT 47
#define SBC_KEYWORD_PI 48
#define SBC_KEYWORD_POINT 49
#define SBC_KEYWORD_POS 50
#define SBC_KEYWORD_RAD 51
#define SBC_KEYWORD_RND 52
#define SBC_KEYWORD_SGN 53
#define SBC_KEYWORD_SIN 54
#define SBC_KEYWORD_SQR 55
#define SBC_KEYWORD_TAN 56
#define SBC_KEYWORD_TO 57
#define SBC_KEYWORD_TRUE 58
#define SBC_KEYWORD_USR 59
#define SBC_KEYWORD_VAL 60
#define SBC_KEYWORD_VPOS 61
#define SBC_KEYWORD_CHR_STR 62
#define SBC_KEYWORD_GET_STR 63
#define SBC_KEYWORD_INKEY_STR 64
#define SBC_KEYWORD_LEFT_STR 65
#define SBC_KEYWORD_MID_STR 66
#define SBC_KEYWORD_RIGHT_STR 67
#define SBC_KEYWORD_STR_STR 68
#define SBC_KEYWORD_STRING_STR 69
#define SBC_KEYWORD_EOF_HASH 70



#define SBC_KEYWORD_WHEN 74
#define SBC_KEYWORD_OF 75
#define SBC_KEYWORD_ENDCASE 76
#define SBC_KEYWORD_ELSE_2 77
#define SBC_KEYWORD_ENDIF 78
#define SBC_KEYWORD_ENDWHILE 79
#define SBC_KEYWORD_PTR_2 80
#define SBC_KEYWORD_PAGE_2 81
#define SBC_KEYWORD_TIME_2 82
#define SBC_KEYWORD_LOMEM_2 83
#define SBC_KEYWORD_HIMEM_2 84
#define SBC_KEYWORD_SOUND 85
#define SBC_KEYWORD_BPUT 86
#define SBC_KEYWORD_CALL 87
#define SBC_KEYWORD_CHAIN 88
#define SBC_KEYWORD_CLEAR 89
#define SBC_KEYWORD_CLOSE 90
#define SBC_KEYWORD_CLG 91
#define SBC_KEYWORD_CLS 92
#define SBC_KEYWORD_DATA 93
#define SBC_KEYWORD_DEF 94
#define SBC_KEYWORD_DIM 95
#define SBC_KEYWORD_DRAW 96
#define SBC_KEYWORD_END 97
#define SBC_KEYWORD_ENDPROC 98
#define SBC_KEYWORD_ENVELOPE 99
#define SBC_KEYWORD_FOR 100
#define SBC_KEYWORD_GOSUB 101
#define SBC_KEYWORD_GOTO 102
#define SBC_KEYWORD_GCOL 103
#define SBC_KEYWORD_IF 104
#define SBC_KEYWORD_INPUT 105
#define SBC_KEYWORD_LET 106
#define SBC_KEYWORD_LOCAL 107
#define SBC_KEYWORD_MODE 108
#define SBC_KEYWORD_MOVE 109
#define SBC_KEYWORD_NEXT 110
#define SBC_KEYWORD_ON 111
#define SBC_KEYWORD_VDU 112
#define SBC_KEYWORD_PLOT 113
#define SBC_KEYWORD_PRINT 114
#define SBC_KEYWORD_PROC 115
#define SBC_KEYWORD_READ 116
#define SBC_KEYWORD_REM 117
#define SBC_KEYWORD_REPEAT 118
#define SBC_KEYWORD_REPORT 119
#define SBC_KEYWORD_RESTORE 120
#define SBC_KEYWORD_RETURN 121
#define SBC_KEYWORD_RUN 122
#define SBC_KEYWORD_STOP 123
#define SBC_KEYWORD_COLOUR 124
#define SBC_KEYWORD_TRACE 125
#define SBC_KEYWORD_UNTIL 126
#define SBC_KEYWORD_WIDTH 127
#define SBC_KEYWORD_OSCLI 128

#define SBC_KEYWORD_CASE 129
#define SBC_KEYWORD_CIRCLE 130
#define SBC_KEYWORD_FILL 131
#define SBC_KEYWORD_ORIGIN 132
#define SBC_KEYWORD_POINT2 133
#define SBC_KEYWORD_RECT 134
#define SBC_KEYWORD_SWAP 135
#define SBC_KEYWORD_WHILE 136
#define SBC_KEYWORD_WAIT 137
#define SBC_KEYWORD_MOUSE 138
#define SBC_KEYWORD_QUIT 139
#define SBC_KEYWORD_SYS 140
#define SBC_KEYWORD_INSTALL 141
#define SBC_KEYWORD_LIBRARY 142
#define SBC_KEYWORD_TINT 143
#define SBC_KEYWORD_ELLIPSE 144
#define SBC_KEYWORD_BEATS 145
#define SBC_KEYWORD_TEMPO 146
#define SBC_KEYWORD_VOICES 147
#define SBC_KEYWORD_VOICE 148
#define SBC_KEYWORD_STEREO 149
#define SBC_KEYWORD_OVERLAY 150

#define SBC_KEYWORD_APPEND 151
#define SBC_KEYWORD_AUTO 152
#define SBC_KEYWORD_CRUNCH 153
#define SBC_KEYWORD_DELETE 154
#define SBC_KEYWORD_EDIT 155
#define SBC_KEYWORD_HELP 156
#define SBC_KEYWORD_LIST 157
#define SBC_KEYWORD_LOAD 158
#define SBC_KEYWORD_LVAR 159
#define SBC_KEYWORD_NEW 160
#define SBC_KEYWORD_OLD 161
#define SBC_KEYWORD_RENUMBER 162
#define SBC_KEYWORD_SAVE 163
#define SBC_KEYWORD_TEXTLOAD 164
#define SBC_KEYWORD_TEXTSAVE 165
#define SBC_KEYWORD_TWIN 166
#define SBC_KEYWORD_TWINNO 167
#define SBC_KEYWORD_INSTALL2 168

#define SBC_KEYWORD_SUM 169
#define SBC_KEYWORD_BEAT 170
#define SBC_KEYWORD_MAX (SBC_KEYWORD_BEAT + 1)

#define SBC_KEYWORD_UNUSED 240

struct sbc_token_t {
	sbc_token_type_t type;
	union {
		uint8_t keyword;
		int32_t integer;
		sbc_real_t real;
		sbc_id_type_t id_type;
		uint16_t line_no;
	} tok;
	uint16_t ptr;
	uint8_t len;
};

typedef struct sbc_token_t sbc_token_t;

struct sbc_lexer_state_t {
	specasm_handle_t h;
	sbc_token_t tok;
	uint8_t lex_buf[SBC_LEX_BUF_SIZE];
	uint16_t start;
	uint16_t end;
	uint8_t eof;
	uint16_t line_no;
};

typedef struct sbc_lexer_state_t sbc_lexer_state_t;

void sbc_lexer_open_e(const char *f);
void sbc_lexer_close(void);
void sbc_lexer_get_token_e();

#endif
