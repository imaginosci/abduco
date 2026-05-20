/* SPDX-License-Identifier: ISC */
#ifndef PACKET_H
#define PACKET_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "debug.h"

#define PACKET_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

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

bool send_packet(int socket, Packet *pkt);
bool recv_packet(int socket, Packet *pkt);

#if defined(NDEBUG) || !defined(DEBUG_FD)
static inline void print_packet(const char *, const Packet *) {
}
#else
static inline void print_packet(const char *prefix, const Packet *pkt) {
	static const char *msgtype[] = {
		[MSG_CONTENT] = "CONTENT",
		[MSG_ATTACH]  = "ATTACH",
		[MSG_DETACH]  = "DETACH",
		[MSG_RESIZE]  = "RESIZE",
		[MSG_EXIT]    = "EXIT",
		[MSG_PID]     = "PID",
		[MSG_STDIN_EOF] = "STDIN_EOF",
	};
	const char *type = "UNKNOWN";
	if (pkt->type < PACKET_COUNT(msgtype) && msgtype[pkt->type])
		type = msgtype[pkt->type];

	dprintf(DEBUG_FD, "%s type=%s(%"PRIu32") len=%"PRIu32,
	        prefix, type, pkt->type, pkt->len);
	switch (pkt->type) {
	case MSG_CONTENT:
		break;
	case MSG_RESIZE:
		dprintf(DEBUG_FD, " cols=%"PRIu16" rows=%"PRIu16,
		        pkt->u.ws.cols, pkt->u.ws.rows);
		break;
	case MSG_ATTACH:
		dprintf(DEBUG_FD, " flags=0x%"PRIx32, pkt->u.i);
		break;
	case MSG_EXIT:
		dprintf(DEBUG_FD, " status=%"PRIu32, pkt->u.i);
		break;
	case MSG_PID:
		dprintf(DEBUG_FD, " pid=%"PRIu64, pkt->u.l);
		break;
	default:
		break;
	}
	dprintf(DEBUG_FD, "\n");
}
#endif

#endif
