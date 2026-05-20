/* SPDX-License-Identifier: ISC */
#ifndef DEBUG_H
#define DEBUG_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(NDEBUG) || !defined(DEBUG_FD)
static inline void debug(const char *, ...) {
}

static inline void debug_errno(const char *, ...) {
}
#else
#define debug(...) dprintf(DEBUG_FD, __VA_ARGS__)

static inline void debug_errno(const char *fmt, ...) {
	int err = errno;
	va_list ap;

	va_start(ap, fmt);
	vdprintf(DEBUG_FD, fmt, ap);
	va_end(ap);
	dprintf(DEBUG_FD, " errno=%d (%s)\n", err, strerror(err));
}
#endif

#endif
