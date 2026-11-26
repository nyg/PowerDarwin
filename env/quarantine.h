/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/* The stubs that copyfile-42's copyfile.c defines for embedded targets, changed on 2026-11-19: moved to a header, made inline, with qtn_xattr_name, qtn_file_init_with_path and qtn_file_init_with_data added; on 2026-11-26, qtn_file_apply_to_path added, which gnutar calls. */

#ifndef _QUARANTINE_H_
#define _QUARANTINE_H_

#include <stddef.h>

#define qtn_xattr_name "com.apple.quarantine"
#define QTN_SERIALIZED_DATA_MAX 0

typedef void *qtn_file_t;

static inline qtn_file_t qtn_file_alloc(void) { return NULL; }
static inline int qtn_file_init_with_fd(qtn_file_t x, int y) { return -1; }
static inline int qtn_file_init_with_path(qtn_file_t x, const char *y) { return -1; }
static inline int qtn_file_init_with_data(qtn_file_t x, const void *y, size_t z) { return -1; }
static inline void qtn_file_free(qtn_file_t x) { return; }
static inline int qtn_file_apply_to_fd(qtn_file_t x, int y) { return -1; }
static inline int qtn_file_apply_to_path(qtn_file_t x, const char *y) { return -1; }
static inline char *qtn_error(int x) { return NULL; }
static inline int qtn_file_to_data(qtn_file_t x, char *y, size_t z) { return -1; }
static inline qtn_file_t qtn_file_clone(qtn_file_t x) { return NULL; }

#endif
