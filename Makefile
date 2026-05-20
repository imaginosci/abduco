-include config.mk

VERSION = 0.7

CFLAGS_STD ?= -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DNDEBUG
CFLAGS_STD += -DVERSION=\"${VERSION}\"
STRICT_CFLAGS = -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes
CFLAGS_DEBUG ?= -U_FORTIFY_SOURCE -UNDEBUG -O0 -g -ggdb -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-sign-compare -DDEBUG_FD=9

ifdef STRICT
CFLAGS_EXTRA += ${STRICT_CFLAGS}
endif

LDFLAGS_STD ?= -lc -lutil

STRIP ?= strip
INSTALL ?= install
GCOV ?= gcov
LCOV ?= lcov
GENHTML ?= genhtml

PREFIX ?= /usr/local
SHAREDIR ?= ${PREFIX}/share
TEST_RUNNER ?= ./tests/run.sh
UNAME_S := $(shell uname)

SRC = abduco.c client.c server.c io.c packet.c session.c
UNIT_TESTS = tests/unit/io_test tests/unit/packet_test

ifeq (${UNAME_S},AIX)
SRC += forkpty-aix.c
endif
ifeq (${UNAME_S},SunOS)
SRC += forkpty-sunos.c
endif

OBJ = ${SRC:.c=.o}

all: abduco

config.h:
	cp config.def.h config.h

config.mk:
	@touch $@

abduco: ${OBJ}
	${CC} ${OBJ} ${LDFLAGS} ${LDFLAGS_STD} ${LDFLAGS_AUTO} ${LDFLAGS_EXTRA} -o $@

.c.o:
	${CC} ${CFLAGS} ${CFLAGS_STD} ${CFLAGS_AUTO} ${CFLAGS_EXTRA} -c $< -o $@

${OBJ}: config.h config.mk abduco.h client.h server.h debug.h io.h packet.h session.h

debug: clean
	${MAKE} CFLAGS_EXTRA='${CFLAGS_DEBUG}'

test: clean test-unit abduco
	${TEST_RUNNER}

test-unit: ${UNIT_TESTS}
	@for test in ${UNIT_TESTS}; do ./$$test || exit 1; done

tests/unit/packet_test: tests/unit/packet_test.c packet.c io.c packet.h io.h
	${CC} ${CFLAGS} ${CFLAGS_STD} ${CFLAGS_AUTO} ${CFLAGS_EXTRA} -I. \
		tests/unit/packet_test.c packet.c io.c \
		${LDFLAGS} ${LDFLAGS_STD} ${LDFLAGS_AUTO} ${LDFLAGS_EXTRA} -o $@

tests/unit/io_test: tests/unit/io_test.c io.c io.h
	${CC} ${CFLAGS} ${CFLAGS_STD} ${CFLAGS_AUTO} ${CFLAGS_EXTRA} -I. \
		tests/unit/io_test.c io.c \
		${LDFLAGS} ${LDFLAGS_STD} ${LDFLAGS_AUTO} ${LDFLAGS_EXTRA} -o $@

coverage-gcov: clean
	${MAKE} CFLAGS_EXTRA='-O0 -g --coverage' LDFLAGS_EXTRA='--coverage' abduco
	${TEST_RUNNER}
	${GCOV} -b -c ${SRC}

coverage-html: coverage-gcov
	${LCOV} --capture --directory . --output-file coverage.info
	${GENHTML} coverage.info --output-directory coverage

coverage: coverage-html

clean:
	@echo cleaning
	@rm -f abduco ${UNIT_TESTS} abduco-*.tar.gz coverage.info typescript *.o
	@rm -f *.gcda *.gcno *.gcov
	@rm -rf coverage

dist: clean
	@echo creating dist tarball
	@git archive --prefix=abduco-${VERSION}/ -o abduco-${VERSION}.tar.gz HEAD

installdirs:
	@${INSTALL} -d ${DESTDIR}${PREFIX}/bin \
		${DESTDIR}${MANPREFIX}/man1

install: abduco installdirs
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@${INSTALL} -m 0755 abduco ${DESTDIR}${PREFIX}/bin
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < abduco.1 > ${DESTDIR}${MANPREFIX}/man1/abduco.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/abduco.1

install-strip: install
	${STRIP} ${DESTDIR}${PREFIX}/bin/abduco

install-completion:
	@echo installing zsh completion file to ${DESTDIR}${SHAREDIR}/zsh/site-functions
	@install -Dm644 contrib/abduco.zsh ${DESTDIR}${SHAREDIR}/zsh/site-functions/_abduco

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/abduco
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/abduco.1
	@echo removing zsh completion file from ${DESTDIR}${SHAREDIR}/zsh/site-functions
	@rm -f ${DESTDIR}${SHAREDIR}/zsh/site-functions/_abduco

.PHONY: all clean dist install installdirs install-strip install-completion uninstall debug test test-unit coverage coverage-gcov coverage-html
