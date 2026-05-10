-include config.mk

VERSION = 0.7

CFLAGS_STD ?= -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DNDEBUG
CFLAGS_STD += -DVERSION=\"${VERSION}\"

LDFLAGS_STD ?= -lc -lutil

STRIP ?= strip
INSTALL ?= install
GCOV ?= gcov
LCOV ?= lcov
GENHTML ?= genhtml

PREFIX ?= /usr/local
SHAREDIR ?= ${PREFIX}/share
TEST_RUNNER ?= ./testsuite.sh

SRC = abduco.c

all: abduco

config.h:
	cp config.def.h config.h

config.mk:
	@touch $@

abduco: config.h config.mk *.c
	${CC} ${CFLAGS} ${CFLAGS_STD} ${CFLAGS_AUTO} ${CFLAGS_EXTRA} ${SRC} ${LDFLAGS} ${LDFLAGS_STD} ${LDFLAGS_AUTO} ${LDFLAGS_EXTRA} -o $@

debug: clean
	${MAKE} CFLAGS_EXTRA='${CFLAGS_DEBUG}'

test: clean abduco
	@if [ "$$(uname)" = "Darwin" ]; then \
		script -q /dev/null sh -c 'cat; ${TEST_RUNNER}' </dev/null; \
	else \
		script -q -e -c "sh -c 'cat; ${TEST_RUNNER}'" /dev/null </dev/null; \
	fi

coverage-gcov: clean
	${MAKE} CFLAGS_EXTRA='-O0 -g --coverage' LDFLAGS_EXTRA='--coverage' abduco
	@if [ "$$(uname)" = "Darwin" ]; then \
		script -q /dev/null sh -c 'cat; ${TEST_RUNNER}' </dev/null; \
	else \
		script -q -e -c 'sh -c "cat; ${TEST_RUNNER}"' /dev/null </dev/null; \
	fi
	${GCOV} -b -c ${SRC}

coverage-html: coverage-gcov
	${LCOV} --capture --directory . --output-file coverage.info
	${GENHTML} coverage.info --output-directory coverage

coverage: coverage-html

clean:
	@echo cleaning
	@rm -f abduco abduco-*.tar.gz coverage.info typescript
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

.PHONY: all clean dist install installdirs install-strip install-completion uninstall debug test coverage coverage-gcov coverage-html
