#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

echo "hello" | "$ABDUCO" -c "$(session_path eof-forward)" cat >/dev/null 2>&1 || fail "piped EOF test failed"
sleep 1
assert_clean_sessions

"$ABDUCO" -c "$(session_path eof-devnull)" cat </dev/null >/dev/null 2>&1 || fail "devnull EOF test failed"
sleep 1
assert_clean_sessions

pass
