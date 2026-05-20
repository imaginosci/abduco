/* SPDX-License-Identifier: ISC */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#if defined(NDEBUG) || !defined(DEBUG_FD)
static inline void debug(const char *, ...) {
}
#else
#define debug(...) dprintf(DEBUG_FD, __VA_ARGS__)
#endif

#endif
