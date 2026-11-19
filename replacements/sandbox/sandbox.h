#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#define SANDBOX_NAMED 0x0001

extern const char kSBXProfileNoInternet[];
extern const char kSBXProfileNoNetwork[];
extern const char kSBXProfileNoWrite[];
extern const char kSBXProfileNoWriteExceptTemporary[];
extern const char kSBXProfilePureComputation[];

int sandbox_init(const char *profile, uint64_t flags, char **errorbuf);
void sandbox_free_error(char *errorbuf);

__END_DECLS

#endif
