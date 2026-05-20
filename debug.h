/* SPDX-License-Identifier: ISC */
#ifndef DEBUG_H
#define DEBUG_H

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>

#include "abduco.h"
#include "packet.h"

#if defined(NDEBUG) || !defined(DEBUG_FD)
static inline void debug(const char *, ...) {
}

static inline void print_packet(const char *, const Packet *) {
}
#else
#define debug(...) dprintf(DEBUG_FD, __VA_ARGS__)

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
	if (pkt->type < countof(msgtype) && msgtype[pkt->type])
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
		dprintf(DEBUG_FD, " readonly=%d low-priority=%d",
		        pkt->u.i & CLIENT_READONLY,
		        pkt->u.i & CLIENT_LOWPRIORITY);
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
