#!/bin/sh

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ABDUCO=${ABDUCO:-"$ROOT/abduco"}

if [ "$#" -gt 0 ]; then
	ABDUCO=$1
	shift
fi

case "$ABDUCO" in
/*) ;;
*) ABDUCO=$(CDPATH= cd -- "$(dirname -- "$ABDUCO")" && pwd)/$(basename -- "$ABDUCO") ;;
esac

export ROOT ABDUCO

if [ -z "$ABDUCO_TEST_TTY" ] && [ ! -t 0 ]; then
	case "$(uname)" in
	Darwin|FreeBSD|DragonFly|NetBSD)
		exec script -q /dev/null env ABDUCO_TEST_TTY=1 ROOT="$ROOT" ABDUCO="$ABDUCO" sh "$0" "$@"
		;;
	OpenBSD)
		exec script -c "ABDUCO_TEST_TTY=1 ROOT='$ROOT' ABDUCO='$ABDUCO' sh '$0' $*" /dev/null
		;;
	*)
		exec script -q -e -c "ABDUCO_TEST_TTY=1 ROOT='$ROOT' ABDUCO='$ABDUCO' sh '$0' $*" /dev/null
		;;
	esac
fi

status=0
ran=0

for test in "$ROOT"/tests/test-*.sh; do
	[ -f "$test" ] || continue
	ran=$((ran + 1))
	if TEST_NAME=$(basename "$test" .sh) sh "$test"; then
		:
	else
		status=1
	fi
done

[ "$ran" -gt 0 ] || {
	echo "not ok - no tests found" >&2
	exit 1
}

exit "$status"
