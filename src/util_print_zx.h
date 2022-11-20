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

#ifndef UTIL_PRINT_ZX_H
#define UTIL_PRINT_ZX_H

#include <stdint.h>

uint8_t specasm_util_print(const char *str, uint8_t x, uint8_t y, uint8_t attr);
void specasm_util_clear(uint8_t x, uint8_t y, uint8_t l, uint8_t attr);

#endif
