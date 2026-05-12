/* SPDX-License-Identifier: ISC */
#ifndef SERVER_H
#define SERVER_H

#include "abduco.h"

int server_create_socket(const char *name);
int server_set_socket_non_blocking(int sock);
void server_pty_died_handler(int sig);
void server_sigterm_handler(int sig);
void server_sigusr1_handler(int sig);
void server_mainloop(void);

#endif
