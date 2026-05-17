// SPDX-License-Identifier: ISC
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "packet.h"

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

static int test_content_roundtrip(void) {
	int fd[2];
	Packet out = {
		.type = MSG_CONTENT,
		.len = 5,
	};
	Packet in;

	memcpy(out.u.msg, "hello", out.len);

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(send_packet(fd[0], &out));
	CHECK(recv_packet(fd[1], &in));
	CHECK(in.type == MSG_CONTENT);
	CHECK(in.len == out.len);
	CHECK(memcmp(in.u.msg, out.u.msg, out.len) == 0);

	close_pair(fd);
	return 0;
}

static int test_empty_packet_roundtrip(void) {
	int fd[2];
	Packet out = {
		.type = MSG_DETACH,
		.len = 0,
	};
	Packet in;

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(send_packet(fd[0], &out));
	CHECK(recv_packet(fd[1], &in));
	CHECK(in.type == MSG_DETACH);
	CHECK(in.len == 0);

	close_pair(fd);
	return 0;
}

static int test_recv_rejects_oversized_packet(void) {
	int fd[2];
	Packet out = {
		.type = MSG_CONTENT,
		.len = sizeof(out.u.msg) + 1,
	};
	Packet in = {
		.len = 1,
	};
	size_t header_size = offsetof(Packet, u);

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	CHECK(write(fd[0], &out, header_size) == (ssize_t)header_size);
	CHECK(!recv_packet(fd[1], &in));
	CHECK(in.len == 0);

	close_pair(fd);
	return 0;
}

static int test_send_rejects_oversized_packet(void) {
	Packet pkt = {
		.type = MSG_CONTENT,
		.len = sizeof(pkt.u.msg) + 1,
	};

	CHECK(!send_packet(-1, &pkt));
	return 0;
}

static int test_recv_reports_eof(void) {
	int fd[2];
	Packet in;

	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0);
	close(fd[0]);
	CHECK(!recv_packet(fd[1], &in));
	close(fd[1]);

	return 0;
}

int main(void) {
	CHECK(test_content_roundtrip() == 0);
	CHECK(test_empty_packet_roundtrip() == 0);
	CHECK(test_recv_rejects_oversized_packet() == 0);
	CHECK(test_send_rejects_oversized_packet() == 0);
	CHECK(test_recv_reports_eof() == 0);

	return 0;
}
