#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

command -v dvtm >/dev/null 2>&1 || skip "dvtm not installed"

dvtm_cmd() {
	printf "\007$1\n"
	sleep 1
}

dvtm_session() {
	sleep 1
	dvtm_cmd 'c'
	dvtm_cmd 'c'
	dvtm_cmd 'c'
	sleep 1
	dvtm_cmd ' '
	dvtm_cmd ' '
	dvtm_cmd ' '
	sleep 1
	dvtm_cmd 'qq'
}

cd "$TEST_TMP" || exit 1

expected="$TEST_TMP/expected"
output="$TEST_TMP/output"
session=$(session_path dvtm)
: > "$expected"

dvtm_session | "$ABDUCO" -c "$session" > "$output" 2>&1
diff_output "$expected" "$output" || fail "unexpected dvtm output"
assert_clean_sessions
pass
