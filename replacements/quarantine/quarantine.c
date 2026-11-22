/* SPDX-License-Identifier: MIT */

#include <stddef.h>

typedef struct qtn_file *qtn_file_t;
typedef struct qtn_proc *qtn_proc_t;

const char _qtn_xattr_name[] = "com.apple.quarantine";
const char _qtn_label_name[] = "q";

const char *_qtn_error(int error)
{
   return "quarantine is not available in this Darwin build";
}

qtn_file_t _qtn_file_alloc(void) { return NULL; }
qtn_file_t _qtn_file_clone(qtn_file_t file) { return NULL; }
void _qtn_file_free(qtn_file_t file) { }
int _qtn_file_init(qtn_file_t file) { return -1; }
int _qtn_file_init_with_fd(qtn_file_t file, int fd) { return -1; }
int _qtn_file_init_with_path(qtn_file_t file, const char *path) { return -1; }
int _qtn_file_init_with_mount_point(qtn_file_t file, const char *path) { return -1; }
int _qtn_file_init_with_data(qtn_file_t file, const void *data, size_t size) { return -1; }
int _qtn_file_to_data(qtn_file_t file, char *data, size_t *size) { return -1; }
int _qtn_file_apply_to_fd(qtn_file_t file, int fd) { return -1; }
int _qtn_file_apply_to_path(qtn_file_t file, const char *path) { return -1; }
int _qtn_file_apply_to_mount_point(qtn_file_t file, const char *path) { return -1; }
const char *_qtn_file_get_identifier(qtn_file_t file) { return NULL; }
int _qtn_file_set_identifier(qtn_file_t file, const char *identifier) { return -1; }
int _qtn_file_get_timestamp(qtn_file_t file, void *timestamp) { return -1; }
int _qtn_file_set_timestamp(qtn_file_t file, const void *timestamp) { return -1; }
size_t _qtn_file_get_metadata_size(qtn_file_t file) { return 0; }
const void *_qtn_file_get_metadata(qtn_file_t file) { return NULL; }
int _qtn_file_set_metadata(qtn_file_t file, const void *metadata, size_t size) { return -1; }

qtn_proc_t _qtn_proc_alloc(void) { return NULL; }
qtn_proc_t _qtn_proc_clone(qtn_proc_t proc) { return NULL; }
void _qtn_proc_free(qtn_proc_t proc) { }
int _qtn_proc_init(qtn_proc_t proc) { return -1; }
int _qtn_proc_init_with_self(qtn_proc_t proc) { return -1; }
int _qtn_proc_init_with_data(qtn_proc_t proc, const void *data, size_t size) { return -1; }
int _qtn_proc_to_data(qtn_proc_t proc, char *data, size_t *size) { return -1; }
int _qtn_proc_apply_to_self(qtn_proc_t proc) { return -1; }
const char *_qtn_proc_get_identifier(qtn_proc_t proc) { return NULL; }
int _qtn_proc_set_identifier(qtn_proc_t proc, const char *identifier) { return -1; }
size_t _qtn_proc_get_metadata_size(qtn_proc_t proc) { return 0; }
const void *_qtn_proc_get_metadata(qtn_proc_t proc) { return NULL; }
int _qtn_proc_set_metadata(qtn_proc_t proc, const void *metadata, size_t size) { return -1; }
const char *_qtn_proc_get_path_exclusion_pattern(qtn_proc_t proc) { return NULL; }
int _qtn_proc_set_path_exclusion_pattern(qtn_proc_t proc, const char *pattern) { return -1; }
