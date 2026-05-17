// SPDX-License-Identifier: ISC
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "io.h"

#define CHECK(expr) do { \
	if (!(expr)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", \
		        __FILE__, __LINE__, #expr); \
		return 1; \
	} \
} while (0)

static void close_pair(int fd[2]) {
	close(fd[0]);
	close(fd[1]);
}

static int test_write_all_writes_full_buffer(void) {
	int fd[2];
	char buf[5];

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(write_all(fd[0], "hello", 5) == 5);
	CHECK(read(fd[1], buf, sizeof(buf)) == (ssize_t)sizeof(buf));
	CHECK(memcmp(buf, "hello", sizeof(buf)) == 0);

	close_pair(fd);
	return 0;
}

static int test_read_all_reads_full_buffer(void) {
	int fd[2];
	char buf[5];

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(write(fd[0], "hello", 5) == 5);
	CHECK(read_all(fd[1], buf, sizeof(buf)) == (ssize_t)sizeof(buf));
	CHECK(memcmp(buf, "hello", sizeof(buf)) == 0);

	close_pair(fd);
	return 0;
}

static int test_read_all_reports_partial_eof(void) {
	int fd[2];
	char buf[5] = { 0 };

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(write(fd[0], "hi", 2) == 2);
	close(fd[0]);
	CHECK(read_all(fd[1], buf, sizeof(buf)) == 2);
	CHECK(memcmp(buf, "hi", 2) == 0);
	close(fd[1]);

	return 0;
}

static int test_read_all_reports_wouldblock_progress(void) {
	int fd[2];
	char buf[5] = { 0 };
	int flags;

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	flags = fcntl(fd[1], F_GETFL);
	CHECK(flags >= 0);
	CHECK(fcntl(fd[1], F_SETFL, flags | O_NONBLOCK) == 0);
	CHECK(write(fd[0], "hi", 2) == 2);
	CHECK(read_all(fd[1], buf, sizeof(buf)) == 2);
	CHECK(memcmp(buf, "hi", 2) == 0);

	close_pair(fd);
	return 0;
}

int main(void) {
	CHECK(test_write_all_writes_full_buffer() == 0);
	CHECK(test_read_all_reads_full_buffer() == 0);
	CHECK(test_read_all_reports_partial_eof() == 0);
	CHECK(test_read_all_reports_wouldblock_progress() == 0);

	return 0;
}
