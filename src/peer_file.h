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

#ifndef SPECASM_PEER_FILE_H
#define SPECASM_PEER_FILE_H

#ifdef SPECTRUM
#include <arch/zx/esxdos.h>
typedef unsigned char specasm_handle_t;
typedef unsigned char specasm_dir_t;
typedef struct esxdos_dirent specasm_dirent_t;
#define SPECASM_PATH_MAX ESXDOS_PATH_MAX
#define specasm_readdir(dir, drent) esxdos_f_readdir(dir, drent)
#define specasm_closedir(dir) esxdos_f_close(dir)
#define specasm_getdirname(d) (&d.dir[1])
#define specasm_remove_file(f) (void)esxdos_f_unlink(f)
#else
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#define SPECASM_PATH_MAX PATH_MAX
typedef FILE *specasm_handle_t;
typedef DIR *specasm_dir_t;
typedef struct dirent specasm_dirent_t;
uint8_t specasm_readdir(specasm_dir_t dir, specasm_dirent_t *dirent);
#define specasm_closedir(dir) (void)closedir(dir)
#define specasm_getdirname(d) d.d_name
#define specasm_remove_file(f) (void)remove(f)
#endif

specasm_handle_t specasm_file_wopen_e(const char *fname);
specasm_handle_t specasm_file_ropen_e(const char *fname);
void specasm_file_write_e(specasm_handle_t f, const void *data, size_t size);
size_t specasm_file_read_e(specasm_handle_t f, void *data, size_t size);
void specasm_file_close_e(specasm_handle_t f);
specasm_dir_t specasm_opendir_e(const char *fname);

#endif
