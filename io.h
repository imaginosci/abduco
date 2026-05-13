/* SPDX-License-Identifier: ISC */
#ifndef IO_H
#define IO_H

#include <sys/types.h>

ssize_t write_all(int fd, const char *buf, size_t len);
ssize_t read_all(int fd, char *buf, size_t len);

#endif
