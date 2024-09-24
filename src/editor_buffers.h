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


#ifndef EDITOR_BUFFERS_H
#define EDITOR_BUFFERS_H

#include "line.h"

#define MAX_FNAME 28

extern char current_fname[MAX_FNAME + 1];
extern char line_buf[SPECASM_MAX_SCRATCH];
extern unsigned int line;
extern uint8_t col;
extern uint8_t quitting;

#endif
