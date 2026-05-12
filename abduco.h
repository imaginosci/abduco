// SPDX-License-Identifier: ISC
/*
 * Copyright (c) 2013-2018 Marc André Tanner <mat at brain-dump.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef ABDUCO_H
#define ABDUCO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#if defined CTRL && defined _AIX
  #undef CTRL
#endif
#ifndef CTRL
  #define CTRL(k)   ((k) & 0x1F)
#endif

#define countof(arr) (sizeof(arr) / sizeof((arr)[0]))

enum PacketType {
	MSG_CONTENT   = 0,
	MSG_ATTACH    = 1,
	MSG_DETACH    = 2,
	MSG_RESIZE    = 3,
	MSG_EXIT      = 4,
	MSG_PID       = 5,
	MSG_STDIN_EOF = 6,
};

typedef struct {
	uint32_t type;
	uint32_t len;
	union {
		char msg[4096 - 2*sizeof(uint32_t)];
		struct {
			uint16_t rows;
			uint16_t cols;
		} ws;
		uint32_t i;
		uint64_t l;
	} u;
} Packet;

typedef struct Client Client;
struct Client {
	int socket;
	enum {
		STATE_CONNECTED,
		STATE_ATTACHED,
		STATE_DETACHED,
		STATE_DISCONNECTED,
	} state;
	bool need_resize;
	enum {
		CLIENT_READONLY = 1 << 0,
		CLIENT_LOWPRIORITY = 1 << 1,
	} flags;
	bool msg_exit_sent;
	Client *next;
};

struct entry {
	char *data;
	int len;
	bool complete;
	TAILQ_ENTRY(entry) entries;
};

TAILQ_HEAD(screenhead, entry);

typedef struct {
	Client *clients;
	int socket;
	int pty;
	int exit_status;
	struct termios term;
	struct winsize winsize;
	struct screenhead screen;
	int screen_rows;
	pid_t pid;
	volatile sig_atomic_t running;
	const char *name;
	const char *session_name;
	char host[255];
	bool read_pty;
} Server;

struct Dir {
	char *path;
	char *env;
	bool personal;
};

extern Server server;
extern Client client;
extern struct termios orig_term, cur_term;
extern bool has_term, alternate_buffer, quiet, passthrough;
extern int screen_max_rows;
extern struct sockaddr_un sockaddr;
extern char KEY_DETACH;
extern char KEY_REDRAW;

ssize_t write_all(int fd, const char *buf, size_t len);
ssize_t read_all(int fd, char *buf, size_t len);
bool send_packet(int socket, Packet *pkt);
bool recv_packet(int socket, Packet *pkt);
bool set_socket_name(struct sockaddr_un *sockaddr, const char *name);
void die(const char *s);
void info(const char *str, ...);

#endif
