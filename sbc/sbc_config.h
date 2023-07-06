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

#ifndef SBC_CONFIG_H
#define SBC_CONFIG_H

#include <stdint.h>

#define SBC_CONFIG_SIZE_SMALL 1
#define SBC_CONFIG_SIZE_MEDIUM 2
#define SBC_CONFIG_SIZE_LARGE 4

#ifdef SPECTRUM
#define SBC_CONFIG_SIZE SBC_CONFIG_SIZE_SMALL
typedef uint8_t sbc_handle_t;
typedef uint16_t sbc_big_handle_t;
#else
#define SBC_CONFIG_SIZE SBC_CONFIG_SIZE_LARGE
typedef unsigned int sbc_handle_t;
typedef unsigned int sbc_big_handle_t;
#endif

struct sbc_real_t {
	uint8_t b[5];
};
typedef struct sbc_real_t sbc_real_t;

#endif
