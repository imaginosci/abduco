/* SPDX-License-Identifier: ISC */
#ifndef SESSION_H
#define SESSION_H

#include <sys/socket.h>
#include <sys/un.h>

#include "abduco.h"

bool session_set_socket_name(const Server *srv, const char *name);
bool session_set_socket_dir(const Server *srv);
int session_socket_bind(int fd);
int session_socket_connect(int fd);
const char *session_socket_path(void);
void session_unlink_socket(void);

#endif
