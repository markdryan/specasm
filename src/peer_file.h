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

#if defined(SPECTRUM) || defined(__ZXNEXT)
typedef unsigned char specasm_handle_t;
typedef unsigned char specasm_dir_t;
#ifdef SPECTRUM
#include <arch/zx/esxdos.h>
typedef struct esxdos_dirent specasm_dirent_t;
typedef struct esxdos_stat specasm_stat_t;
#define SPECASM_PATH_MAX ESXDOS_PATH_MAX
#define specasm_readdir(dir, drent) esxdos_f_readdir(dir, drent)
#define specasm_closedir(dir) esxdos_f_close(dir)
#define specasm_getdirname(d) (&d.dir[1])
#define specasm_remove_file(f) (void)esxdos_f_unlink(f)
#define specasm_get_file_size(stat_buf) (stat_buf)->size
#define specasm_isdirent_dir(d) ((d).dir[0] & __esx_dir_a_dir)
#else
#include <arch/zxn/esxdos.h>
typedef struct esx_dirent specasm_dirent_t;
typedef struct esx_stat specasm_stat_t;
#define SPECASM_PATH_MAX ESX_PATHNAME_MAX
#define specasm_readdir(dir, drent) esx_f_readdir(dir, drent)
#define specasm_closedir(dir) esx_f_close(dir)
#define specasm_getdirname(d) ((d).name)
#define specasm_remove_file(f) (void)esx_f_unlink(f)
#define specasm_get_file_size(stat_buf) (stat_buf)->size
#define specasm_isdirent_dir(d) ((d).attr & ESX_DIR_A_DIR)
#endif

#else

#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#define SPECASM_PATH_MAX PATH_MAX
typedef FILE *specasm_handle_t;
typedef DIR *specasm_dir_t;
typedef struct dirent specasm_dirent_t;
typedef struct stat specasm_stat_t;
uint8_t specasm_readdir(specasm_dir_t dir, specasm_dirent_t *dirent);
#define specasm_closedir(dir) (void)closedir(dir)
#define specasm_getdirname(d) d.d_name
#define specasm_remove_file(f) (void)remove(f)
#define specasm_f_stat(f, stat_buf) fstat(f, stat_buf)
#define specasm_get_file_size(stat_buf) (stat_buf)->st_size
#define specasm_isdirent_dir(d) ((d).d_type & DT_DIR)
#endif

specasm_handle_t specasm_file_wopen_e(const char *fname);
specasm_handle_t specasm_file_ropen_e(const char *fname);
void specasm_file_write_e(specasm_handle_t f, const void *data, size_t size);
size_t specasm_file_read_e(specasm_handle_t f, void *data, size_t size);
void specasm_file_close_e(specasm_handle_t f);
specasm_dir_t specasm_opendir_e(const char *fname);
void specasm_file_stat_e(specasm_handle_t f, specasm_stat_t *buf);
uint8_t specasm_file_isdir(const char *fname);

#endif
