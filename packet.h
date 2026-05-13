/* SPDX-License-Identifier: ISC */
#ifndef PACKET_H
#define PACKET_H

#include <stdbool.h>
#include <stdint.h>

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

#endif
