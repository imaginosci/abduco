#!/bin/sh
. "$(dirname -- "$0")/lib.sh"

cd "$TEST_TMP" || exit 1

session=$(session_path "stall-client")

# Start abduco with a child that writes a large buffer
"$ABDUCO" -c "$session" sh -c 'sleep 0.5; head -c 2000000 /dev/zero | tr "\0" a; sleep 3' >/dev/null 2>&1 &
cpid=$!

# Let child start writing and stop the client process
sleep 0.8
kill -STOP "$cpid" 2>/dev/null

# Verify the session is still responsive to detect/status with portable timeout
( "$ABDUCO" -d "$session" >/dev/null 2>&1 ) &
dpid=$!
( sleep 3; kill -9 "$dpid" 2>/dev/null ) &
wpid=$!

wait "$dpid"
detect_rc=$?

kill "$wpid" 2>/dev/null
wait "$wpid" 2>/dev/null

kill -CONT "$cpid" 2>/dev/null
kill -9 "$cpid" 2>/dev/null
wait "$cpid" 2>/dev/null

[ "$detect_rc" -eq 0 ] || fail "server became unresponsive during client stall (rc=$detect_rc)"

assert_clean_sessions
pass
