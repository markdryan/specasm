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

#ifndef SBC_EXP_H
#define SBC_EXP_H

#include <stdint.h>

#include "sbc_config.h"
#include "sbc_lexer.h"

/*
 * Expression types.  Values are chosen so that an expression type for
 * a leaf value or a simple operator can be derived directly by assignment.
 */

#define SBC_EXP_STRING SBC_TOKEN_STRING      // 4
#define SBC_EXP_INTEGER SBC_TOKEN_INTEGER    // 5
#define SBC_EXP_HEX SBC_TOKEN_HEX            // 6
#define SBC_EXP_BIN SBC_TOKEN_BIN            // 7
#define SBC_EXP_REAL SBC_TOKEN_REAL          // 8
#define SBC_EXP_IDENTIFIER SBC_TOKEN_IDENTIFIER  // 9

#define SBC_EXP_DIV 10
#define SBC_EXP_MOD 11
#define SBC_EXP_GTE 12
#define SBC_EXP_LTE 13
#define SBC_EXP_NEQ 14
#define SBC_EXP_LSL 15
#define SBC_EXP_LSR 16
#define SBC_EXP_ASR 17
#define SBC_EXP_PLUSEQ 18
#define SBC_EXP_MINUSEQ 19
#define SBC_EXP_PEEKW '!'   // 33
#define SBC_EXP_OPENB  '('  // 40
#define SBC_EXP_CLOSEB ')'  // 41
#define SBC_EXP_MUL   '*'   // 42
#define SBC_EXP_PLUS  '+'   // 43
#define SBC_EXP_MINUS '-'   // 45
#define SBC_EXP_RDIV  '/'   // 47
#define SBC_EXP_LT    '<'   // 60
#define SBC_EXP_EQ    '='   // 61
#define SBC_EXP_GT    '>'   // 62
#define SBC_EXP_PEEKB '?'   // 63
#define SBC_EXP_RND 127
#define SBC_EXP_TIME 128
#define SBC_EXP_FN 129
#define SBC_EXP_GET 130
#define SBC_EXP_COS 131
#define SBC_EXP_SIN 132
#define SBC_EXP_SQR 133
#define SBC_EXP_PI 134

#define SBC_POOL_MAX_STRING_BUF (1024 * SBC_CONFIG_SIZE)
extern uint8_t sbc_pool_strings[SBC_POOL_MAX_STRING_BUF + 1];

struct sbc_ast_args_t {
	sbc_handle_t a1;
	sbc_handle_t a2;
};
typedef struct sbc_ast_args_t sbc_ast_args_t;

struct sbc_expression_node_t {
	sbc_handle_t e;

	/*
	 * Points to an expression node.  Set to SBC_MAX_EXP_NODES
	 * if this is the last element.
	 */

	sbc_handle_t next;
};
typedef struct sbc_expression_node_t sbc_expression_node_t;

struct sbc_fn_call_t {
	sbc_big_handle_t name;
	sbc_handle_t args_list;
};
typedef struct sbc_fn_call_t sbc_fn_call_t;

struct sbc_id_t {
	uint8_t id_type;
	sbc_big_handle_t str;
};
typedef struct sbc_id_t sbc_id_t;

/*
 * I spent a lot of time thinking about the layout of this structure.
 * Originally, I was planning to have separate pools for ints and
 * floats and lists, but in the end it just made everything more complicated,
 * didn't save a lot of space and added more points of failure.  So we're
 * just going to bundle everything up together into one structure with a big
 * union.
 */

struct sbc_expression_t {
	uint8_t type;
	union {
		sbc_id_t id;
		int32_t integer;
		sbc_real_t real;
		sbc_fn_call_t fn_call;
		sbc_ast_args_t args;
	} v;
};
typedef struct sbc_expression_t sbc_expression_t;

#define SBC_MAX_EXPRESSIONS (256 * SBC_CONFIG_SIZE)
#define SBC_MAX_EXP_NODES   (255 * SBC_CONFIG_SIZE)

extern sbc_expression_t sbc_expressions[SBC_MAX_EXPRESSIONS];
extern sbc_expression_node_t exp_list[SBC_MAX_EXP_NODES];

uint8_t sbc_exp_map_op(void);
sbc_handle_t sbc_exp_add_int_e(sbc_token_t *t);
sbc_handle_t sbc_exp_add_real_e(sbc_token_t *t);
sbc_handle_t sbc_exp_add_id_base_e(sbc_token_t *t, uint8_t id_type);
sbc_big_handle_t sbc_pool_add_string_e(const uint8_t *v, uint8_t len);
#define sbc_exp_add_id_e(t) sbc_exp_add_id_base_e((t), (t)->tok.id_type);
#define sbc_exp_add_string_e(t) sbc_exp_add_id_base_e((t), SBC_ID_TYPE_NONE)

uint8_t sbc_exp_is_simple_op(uint8_t op);
sbc_handle_t sbc_exp_parse_e(void);
sbc_handle_t sbc_exp_parse_no_get_e(void);
sbc_handle_t sbc_exp_get_node_e(void);
sbc_handle_t sbc_parse_node_list_e(void);
sbc_handle_t sbc_parse_bracketednode_list_e(void);

#endif
