#pragma once

#include <errno.h>

typedef int Errno;

#define DUMMY_ERRNO ((Errno)0xAAAAAAAA)

void exit_on_errno(Errno err);
void exit_on_errno_or(Errno err, const char* unknown_msg);
