// SPDX-License-Identifier: ISC
#include <stddef.h>
#include <stdint.h>

#include "io.h"
#include "packet.h"

static inline size_t packet_header_size(void) {
	return offsetof(Packet, u);
}

static size_t packet_size(Packet *pkt) {
	return packet_header_size() + pkt->len;
}

bool send_packet(int socket, Packet *pkt) {
	size_t size = packet_size(pkt);
	if (size > sizeof(*pkt))
		return false;
	ssize_t written = write_all(socket, (char *)pkt, size);
	return written >= 0 && (size_t)written == size;
}

bool recv_packet(int socket, Packet *pkt) {
	ssize_t len = read_all(socket, (char*)pkt, packet_header_size());
	if (len <= 0 || (size_t)len != packet_header_size())
		return false;
	if (pkt->len > sizeof(pkt->u.msg)) {
		pkt->len = 0;
		return false;
	}
	if (pkt->len > 0) {
		len = read_all(socket, pkt->u.msg, pkt->len);
		if (len <= 0 || (uint32_t)len != pkt->len)
			return false;
	}
	return true;
}
