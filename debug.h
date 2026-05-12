/* SPDX-License-Identifier: ISC */
#ifndef DEBUG_H
#define DEBUG_H

#include "abduco.h"

void debug(const char *errstr, ...);
void print_packet(const char *prefix, Packet *pkt);

#endif
