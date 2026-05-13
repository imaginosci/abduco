// SPDX-License-Identifier: ISC
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "abduco.h"

struct Dir {
	char *path;
	char *env;
	bool personal;
};

#include "session.h"

static struct sockaddr_un sockaddr = {
	.sun_family = AF_UNIX,
};

static struct Dir socket_dirs[] = {
	{ .env  = "ABDUCO_SOCKET_DIR", false },
	{ .env  = "HOME",              true  },
	{ .env  = "TMPDIR",            false },
	{ .path = "/tmp",              false },
};

static bool xsnprintf(char *buf, size_t size, const char *fmt, ...) {
	va_list ap;
	if (size > INT_MAX)
		return false;
	va_start(ap, fmt);
	int n = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	if (n == -1)
		return false;
	if ((size_t)n >= size) {
		errno = ENAMETOOLONG;
		return false;
	}
	return true;
}

static bool create_socket_dir(const Server *srv, struct sockaddr_un *sockaddr) {
	sockaddr->sun_path[0] = '\0';
	int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socketfd == -1)
		return false;

	const size_t maxlen = sizeof(sockaddr->sun_path);
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);

	for (unsigned int i = 0; i < countof(socket_dirs); i++) {
		struct stat sb;
		struct Dir *dir = &socket_dirs[i];
		bool ishome = false;
		if (dir->env) {
			dir->path = getenv(dir->env);
			ishome = !strcmp(dir->env, "HOME");
			if (ishome && (!dir->path || !dir->path[0]) && pw)
				dir->path = pw->pw_dir;
		}
		if (!dir->path || !dir->path[0])
			continue;
		if (!xsnprintf(sockaddr->sun_path, maxlen, "%s/%s%s/", dir->path, ishome ? "." : "", srv->name))
			continue;
		mode_t mask = umask(0);
		int r = mkdir(sockaddr->sun_path, dir->personal ? S_IRWXU : S_IRWXU|S_IRWXG|S_IRWXO|S_ISVTX);
		umask(mask);
		if (r != 0 && errno != EEXIST)
			continue;
		if (lstat(sockaddr->sun_path, &sb) != 0)
			continue;
		if (!S_ISDIR(sb.st_mode)) {
			errno = ENOTDIR;
			continue;
		}

		size_t dirlen = strlen(sockaddr->sun_path);
		if (!dir->personal) {
			/* create subdirectory only accessible to user */
			if (pw && !xsnprintf(sockaddr->sun_path+dirlen, maxlen-dirlen, "%s/", pw->pw_name))
				continue;
			if (!pw && !xsnprintf(sockaddr->sun_path+dirlen, maxlen-dirlen, "%d/", uid))
				continue;
			if (mkdir(sockaddr->sun_path, S_IRWXU) != 0 && errno != EEXIST)
				continue;
			if (lstat(sockaddr->sun_path, &sb) != 0)
				continue;
			if (!S_ISDIR(sb.st_mode)) {
				errno = ENOTDIR;
				continue;
			}
			dirlen = strlen(sockaddr->sun_path);
		}

		if (sb.st_uid != uid || sb.st_mode & (S_IRWXG|S_IRWXO)) {
			errno = EACCES;
			continue;
		}

		if (!xsnprintf(sockaddr->sun_path+dirlen, maxlen-dirlen, ".abduco-%d", getpid()))
			continue;

		socklen_t socklen = offsetof(struct sockaddr_un, sun_path) + strlen(sockaddr->sun_path) + 1;
		if (bind(socketfd, (struct sockaddr*)sockaddr, socklen) == -1)
			continue;
		unlink(sockaddr->sun_path);
		close(socketfd);
		sockaddr->sun_path[dirlen] = '\0';
		return true;
	}

	close(socketfd);
	return false;
}

static bool set_socket_name(const Server *srv, struct sockaddr_un *sockaddr, const char *name) {
	const size_t maxlen = sizeof(sockaddr->sun_path);
	const char *session_name = NULL;
	char buf[maxlen];

	if (name[0] == '/') {
		if (strlen(name) >= maxlen) {
			errno = ENAMETOOLONG;
			return false;
		}
		strncpy(sockaddr->sun_path, name, maxlen);
	} else if (name[0] == '.' && (name[1] == '.' || name[1] == '/')) {
		char *cwd = getcwd(buf, sizeof buf);
		if (!cwd)
			return false;
		if (!xsnprintf(sockaddr->sun_path, maxlen, "%s/%s", cwd, name))
			return false;
	} else {
		if (!create_socket_dir(srv, sockaddr))
			return false;
		if (strlen(sockaddr->sun_path) + strlen(name) + strlen(srv->host) >= maxlen) {
			errno = ENAMETOOLONG;
			return false;
		}
		session_name = name;
		strncat(sockaddr->sun_path, name, maxlen - strlen(sockaddr->sun_path) - 1);
		strncat(sockaddr->sun_path, srv->host, maxlen - strlen(sockaddr->sun_path) - 1);
	}

	if (!session_name) {
		strncpy(buf, sockaddr->sun_path, sizeof buf);
		session_name = basename(buf);
	}
	setenv("ABDUCO_SESSION", session_name, 1);
	setenv("ABDUCO_SOCKET", sockaddr->sun_path, 1);

	return true;
}

bool session_set_socket_name(const Server *srv, const char *name) {
	if (!srv)
		return false;
	return set_socket_name(srv, &sockaddr, name);
}

bool session_set_socket_dir(const Server *srv) {
	if (!srv)
		return false;
	return create_socket_dir(srv, &sockaddr);
}

int session_socket_bind(int fd) {
	socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(sockaddr.sun_path) + 1;
	return bind(fd, (struct sockaddr*)&sockaddr, len);
}

int session_socket_connect(int fd) {
	socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(sockaddr.sun_path) + 1;
	return connect(fd, (struct sockaddr*)&sockaddr, len);
}

const char *session_socket_path(void) {
	return sockaddr.sun_path;
}

void session_unlink_socket(void) {
	unlink(sockaddr.sun_path);
}
