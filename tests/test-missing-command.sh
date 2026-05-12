#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

"$ABDUCO" -c "$(session_path missing)" ./non-existing-command >/dev/null 2>&1
assert_clean_sessions
pass
