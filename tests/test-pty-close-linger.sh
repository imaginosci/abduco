#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

session=$(session_path "pty-close-linger")

# Child process closes all pty descriptors and lingers before exiting with code 7
"$ABDUCO" -c "$session" sh -c 'exec </dev/null >/dev/null 2>&1; sleep 2; exit 7' >/dev/null 2>&1 &
cpid=$!

# Portable watchdog: kill after 8 seconds if hung
( sleep 8; kill -9 "$cpid" 2>/dev/null ) &
wpid=$!

# Measure server CPU usage while child is lingering with closed pty
sleep 0.8
spid=$(pgrep -f "$ABDUCO -c $session" | while read -r p; do
	[ "$p" != "$cpid" ] && echo "$p"
done | head -n 1)

cpu_check_ok=1
if [ -n "$spid" ] && [ -r "/proc/$spid/stat" ]; then
	t1=$(awk '{print $14 + $15}' "/proc/$spid/stat" 2>/dev/null)
	sleep 0.8
	t2=$(awk '{print $14 + $15}' "/proc/$spid/stat" 2>/dev/null)
	hz=$(getconf CLK_TCK 2>/dev/null || echo 100)
	if [ -n "$t1" ] && [ -n "$t2" ]; then
		cpu_spent=$(awk -v a="$t1" -v b="$t2" -v h="$hz" 'BEGIN { printf "%.2f", (b-a)/h }')
		# Buggy server burns ~0.8s of CPU (100% spin); fixed server uses 0.00s.
		# Allow up to 0.20s for system overhead.
		if awk -v c="$cpu_spent" 'BEGIN { exit (c > 0.20 ? 0 : 1) }'; then
			cpu_check_ok=0
		fi
	fi
fi

wait "$cpid"
rc=$?

kill "$wpid" 2>/dev/null
wait "$wpid" 2>/dev/null

[ "$cpu_check_ok" -eq 1 ] || fail "server CPU busy-spin detected ($cpu_spent s spent in 0.8s window)"
[ "$rc" -eq 7 ] || fail "pty close linger test failed with rc=$rc (expected 7)"

assert_clean_sessions
pass
