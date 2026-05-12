#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

run_attached() {
	local name="$1"
	local cmd="$2"
	local session
	local expected="$TEST_TMP/$name.expected"
	local output="$TEST_TMP/$name.output"
	session=$(session_path "$name")

	expected_abduco_prolog > "$expected"
	eval "$cmd" >> "$expected" 2>&1
	expected_abduco_epilog "$session" $? >> "$expected"

	eval "\"$ABDUCO\" -c \"$session\" $cmd" 2>&1 | sed 's/.$//' > "$output"
	diff_output "$expected" "$output" || fail "$name attached output mismatch"
	assert_clean_sessions
}

run_detached() {
	local name="$1"
	local cmd="$2"
	local session
	local expected="$TEST_TMP/$name.expected"
	local output="$TEST_TMP/$name.output"
	session=$(session_path "$name")

	expected_abduco_prolog > "$expected"
	eval "$cmd" >/dev/null 2>&1
	expected_abduco_epilog "$session" $? >> "$expected"

	eval "\"$ABDUCO\" -n \"$session\" $cmd" >/dev/null 2>&1 || fail "$name create failed"
	sleep 1
	"$ABDUCO" -a "$session" 2>&1 | sed 's/.$//' > "$output"
	diff_output "$expected" "$output" || fail "$name detached output mismatch"
	assert_clean_sessions
}

run_attached awk-attached "awk 'BEGIN {for(i=1;i<=1000;i++) print i}'"
run_detached awk-detached "awk 'BEGIN {for(i=1;i<=1000;i++) print i}'"
run_attached false-attached false
run_detached false-detached false
run_attached true-attached true
run_detached true-detached true

pass
