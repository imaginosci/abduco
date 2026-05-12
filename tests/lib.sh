ROOT=${ROOT:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}
ABDUCO=${ABDUCO:-"$ROOT/abduco"}
ABDUCO_OPTS="-e ^\\"

[ -x "$ABDUCO" ] || {
	echo "missing executable: $ABDUCO" >&2
	exit 1
}

TEST_NAME=${TEST_NAME:-$(basename "$0" .sh)}
TEST_TMP=${TEST_TMP:-$(mktemp -d "${TMPDIR:-/tmp}/abduco-test.XXXXXX")}
TEST_SOCKET_TMP=${TEST_SOCKET_TMP:-$(mktemp -d "/tmp/abd.XXXXXX")}
TEST_SESSION_DIR="$TEST_SOCKET_TMP/s"
ABDUCO_SOCKET_DIR="$TEST_SOCKET_TMP/e"
export ABDUCO_SOCKET_DIR
mkdir -p "$ABDUCO_SOCKET_DIR" "$TEST_SESSION_DIR"

cleanup() {
	# Tests should normally consume finished sessions by reattaching.  This is
	# only a last resort for failures, scoped by the private socket directory.
	for socket in "$ABDUCO_SOCKET_DIR"/abduco/"${USER:-$(id -un)}"/*; do
		[ -e "$socket" ] || continue
		[ -S "$socket" ] || continue
		"$ABDUCO" -a "$socket" >/dev/null 2>&1 || true
	done
	for socket in "$TEST_SESSION_DIR"/*; do
		[ -e "$socket" ] || continue
		[ -S "$socket" ] || continue
		"$ABDUCO" -a "$socket" >/dev/null 2>&1 || true
	done
	rm -rf "$TEST_TMP"
	rm -rf "$TEST_SOCKET_TMP"
}

trap cleanup EXIT INT TERM HUP

pass() {
	echo "ok - $TEST_NAME"
}

fail() {
	echo "not ok - $TEST_NAME: $*" >&2
	exit 1
}

skip() {
	echo "skip - $TEST_NAME: $*"
	exit 0
}

session_path() {
	printf "%s/%s\n" "$TEST_SESSION_DIR" "$1"
}

detach() {
	sleep 1
	printf "\034"
}

expected_abduco_prolog() {
	printf "\033[?1049h\033[H"
}

expected_abduco_epilog() {
	echo "\033[?25h\033[?1049labduco: $1: session terminated with exit status $2"
}

normalize_output() {
	TEST_SESSION_DIR=$TEST_SESSION_DIR perl -0pe '
		$dir = $ENV{TEST_SESSION_DIR};
		s/\x00//g;
		s/\x04\x08\x08//g;
		s{[^:\n]+: (\Q$dir\E/[^:\n]+: session terminated)}{abduco: $1}g;
	' "$1"
}

diff_output() {
	local expected="$1"
	local actual="$2"
	local exp_norm="$TEST_TMP/expected.norm"
	local out_norm="$TEST_TMP/output.norm"

	normalize_output "$expected" > "$exp_norm"
	normalize_output "$actual" > "$out_norm"

	if ! diff -u "$exp_norm" "$out_norm"; then
		return 1
	fi
	return 0
}

assert_clean_sessions() {
	local count
	count=$("$ABDUCO" | wc -l)
	[ "$count" -le 1 ] || fail "abduco session left behind"
	for socket in "$TEST_SESSION_DIR"/*; do
		[ -e "$socket" ] || continue
		[ -S "$socket" ] || fail "absolute socket left behind: $socket"
	done
}

wait_for_session() {
	local name="$1"
	local tries=10

	while [ "$tries" -gt 0 ]; do
		if "$ABDUCO" -d "$name"; then
			return 0
		fi
		tries=$((tries - 1))
		sleep 1
	done

	return 1
}
