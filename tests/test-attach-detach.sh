#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

cat > long-running.sh <<-EOF
	#!/bin/sh
	echo Start
	date
	sleep 3
	echo Hello World
	sleep 3
	echo End
	date
	exit 1
EOF
chmod +x long-running.sh

expected="$TEST_TMP/expected"
output="$TEST_TMP/output"
session=$(session_path attach-detach)

./long-running.sh >/dev/null 2>&1
expected_abduco_epilog "$session" $? > "$expected"

if ! detach | "$ABDUCO" $ABDUCO_OPTS -c "$session" ./long-running.sh >/dev/null 2>&1; then
	fail "failed to create and detach session"
fi

sleep 3
"$ABDUCO" -a "$session" 2>&1 | tail -1 | sed 's/.$//' > "$output"
diff_output "$expected" "$output" || fail "unexpected attach output"
assert_clean_sessions
pass
