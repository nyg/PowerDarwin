#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "sandbox.h"

const char kSBXProfileNoInternet[] = "no-internet";
const char kSBXProfileNoNetwork[] = "no-network";
const char kSBXProfileNoWrite[] = "no-write";
const char kSBXProfileNoWriteExceptTemporary[] = "no-write-except-temporary";
const char kSBXProfilePureComputation[] = "pure-computation";

int sandbox_init(const char *profile, uint64_t flags, char **errorbuf)
{
   if (errorbuf != NULL)
      *errorbuf = strdup("sandbox is not available in this Darwin build");
   errno = ENOTSUP;
   return -1;
}

void sandbox_free_error(char *errorbuf)
{
   free(errorbuf);
}
