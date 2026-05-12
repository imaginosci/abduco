#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

cat > exit-status.sh <<-EOF
	#!/bin/sh
	exit 42
EOF
chmod +x exit-status.sh

for mode in attached detached; do
	name="exit-status-$mode"
	session=$(session_path "$name")
	expected="$TEST_TMP/$name.expected"
	output="$TEST_TMP/$name.output"
	expected_abduco_prolog > "$expected"
	expected_abduco_epilog "$session" 42 >> "$expected"

	case "$mode" in
	attached)
		"$ABDUCO" -c "$session" ./exit-status.sh 2>&1 | sed 's/.$//' > "$output"
		;;
	detached)
		"$ABDUCO" -n "$session" ./exit-status.sh >/dev/null 2>&1 || fail "$name create failed"
		sleep 1
		"$ABDUCO" -a "$session" 2>&1 | sed 's/.$//' > "$output"
		;;
	esac

	diff_output "$expected" "$output" || fail "$name output mismatch"
	assert_clean_sessions
done

pass
