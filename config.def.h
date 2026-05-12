/* SPDX-License-Identifier: ISC */
/* default command to execute if non is given and $ABDUCO_CMD is unset */
#define ABDUCO_CMD "dvtm"
/* default detach key, can be overriden at run time using -e option */
static char KEY_DETACH = CTRL('\\');
/* redraw key to send a SIGWINCH signal to underlying process
 * (set to 0 to disable the redraw key) */
static char KEY_REDRAW = 0;
/* Where to place the "abduco" directory storing all session socket files.
 * The first directory to succeed is used. */
static struct Dir socket_dirs[] = {
	{ .env  = "ABDUCO_SOCKET_DIR", false },
	{ .env  = "HOME",              true  },
	{ .env  = "TMPDIR",            false },
	{ .path = "/tmp",              false },
};
