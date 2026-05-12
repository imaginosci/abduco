/* SPDX-License-Identifier: ISC */
#ifndef CLIENT_H
#define CLIENT_H

#include "abduco.h"

void client_sigwinch_handler(int sig);
bool client_recv_packet(Packet *pkt);
void client_restore_terminal(void);
void client_setup_terminal(void);
int client_mainloop(void);

#endif
