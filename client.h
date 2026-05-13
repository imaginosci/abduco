/* SPDX-License-Identifier: ISC */
#ifndef CLIENT_H
#define CLIENT_H

#include "abduco.h"

void client_sigwinch_handler(int sig);
bool client_recv_packet(Server *srv, Packet *pkt);
void client_add_flags(int flags);
void client_set_keys(char detach_key, char redraw_key);
void client_set_passthrough(bool passthrough);
struct termios *client_capture_terminal(void);
void client_restore_terminal(void);
void client_setup_terminal(void);
int client_mainloop(Server *srv);

#endif
