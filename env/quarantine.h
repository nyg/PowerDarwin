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
static inline char *qtn_error(int x) { return NULL; }
static inline int qtn_file_to_data(qtn_file_t x, char *y, size_t z) { return -1; }
static inline qtn_file_t qtn_file_clone(qtn_file_t x) { return NULL; }

#endif
