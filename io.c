// SPDX-License-Identifier: ISC
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <unistd.h>

#include "io.h"

ssize_t write_all(int fd, const char *buf, size_t len) {
	size_t remaining = len;
	while (remaining > 0) {
		ssize_t res = write(fd, buf, remaining);
		if (res < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				struct pollfd pfd = { .fd = fd, .events = POLLOUT };
				int pret;
				while ((pret = poll(&pfd, 1, 1000)) < 0 && errno == EINTR);
				if (pret > 0 && (pfd.revents & (POLLOUT | POLLHUP | POLLERR)))
					continue;
				if (pret == 0)
					errno = ETIMEDOUT;
			}
			return len > remaining ? (ssize_t)(len - remaining) : -1;
		}
		if (res == 0)
			return (ssize_t)(len - remaining);
		buf += res;
		remaining -= (size_t)res;
	}
	return (ssize_t)len;
}

ssize_t read_all(int fd, char *buf, size_t len) {
	size_t remaining = len;
	while (remaining > 0) {
		ssize_t res = read(fd, buf, remaining);
		if (res < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if (len > remaining)
					return (ssize_t)(len - remaining);
				struct pollfd pfd = { .fd = fd, .events = POLLIN };
				int pret;
				while ((pret = poll(&pfd, 1, 1000)) < 0 && errno == EINTR);
				if (pret > 0 && (pfd.revents & (POLLIN | POLLHUP | POLLERR)))
					continue;
				if (pret == 0)
					errno = ETIMEDOUT;
			}
			return len > remaining ? (ssize_t)(len - remaining) : -1;
		}
		if (res == 0)
			return (ssize_t)(len - remaining);
		buf += res;
		remaining -= (size_t)res;
	}
	return (ssize_t)len;
}
