#!/bin/sh
set -eu

# The VS Code server and stop task communicate through a private Linux FIFO.
# A lifetime lock identifies the active supervisor without publishing any PID.
root=$(CDPATH= cd "$(dirname "$0")/.." && pwd -P)
uid=$(id -u)
runtime=${XDG_RUNTIME_DIR:-/tmp}/example-project-debug-$uid
scope=$(printf '%s\n' "$root" "$(readlink /proc/self/ns/pid)" "$(readlink /proc/self/ns/net)" | sha256sum)
state=$runtime/${scope%% *}
fifo=$state/stop.fifo
url=http://127.0.0.1:8080/

fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

umask 077
mkdir "$runtime" 2>/dev/null || :
[ ! -L "$runtime" ] && [ "$(stat -c '%u:%a' "$runtime")" = "$uid:700" ] ||
    fail "Unsafe debug server runtime directory: $runtime"
mkdir -p "$state"

exec 7>>"$state/control.lock"
exec 9>>"$state/server.lock"
# Serialize publishing the FIFO and sending stop requests. A new server cannot
# start until the stop task has observed the previous supervisor exit.
flock -w 5 7 || fail 'Debug server control is busy.'
case "${1:-}" in
    stop)
        [ "$#" -eq 1 ] || fail 'Usage: wait-for-webfsd.sh stop'
        if flock -n 9; then
            exit 0
        fi
        printf q | dd of="$fifo" oflag=nonblock conv=nocreat status=none 2>/dev/null || :
        flock -w 5 9 || fail 'The debug server did not stop within five seconds.'
        exit 0
        ;;
    serve)
        shift
        [ "$#" -gt 0 ] || fail 'Usage: wait-for-webfsd.sh serve webfsd [options]'
        ;;
    *)
        fail 'Usage: wait-for-webfsd.sh {serve webfsd [options]|stop}'
        ;;
esac

command -v curl >/dev/null || fail 'Server readiness requires curl.'
flock -n 9 || fail 'The debug server for this workspace is already running.'
rm -f "$fifo"
mkfifo "$fifo"
exec 8<>"$fifo"

server_pid=
child_running() {
    [ -n "$server_pid" ] && awk -v parent="$$" '
        $1 == "PPid:" { owned = ($2 == parent) }
        $1 == "State:" { running = ($2 != "Z" && $2 != "X") }
        END { exit !(owned && running) }
    ' "/proc/$server_pid/status" 2>/dev/null
}

cleanup() {
    if child_running; then
        kill -TERM "$server_pid" 2>/dev/null || :
        tries=0
        while [ "$tries" -lt 20 ] && child_running; do
            sleep 0.1
            tries=$((tries + 1))
        done
        if child_running; then
            kill -KILL "$server_pid" 2>/dev/null || :
        fi
    fi
    if [ -n "$server_pid" ]; then
        wait "$server_pid" 2>/dev/null || :
    fi
    rm -f "$fifo"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
trap 'exit 129' HUP

printf '%s\n' 'DEV_SERVER_STARTING'
( exec 7>&- 8>&- 9>&-; exec "$@" ) &
server_pid=$!
flock -u 7

owns_listener() {
    # Match the child's socket inodes against IPv4 LISTEN entries on port 8080.
    readlink /proc/"$server_pid"/fd/* 2>/dev/null |
        awk '
            FILENAME == "-" {
                if ($0 ~ /^socket:\[[0-9]+\]$/) {
                    sub(/^socket:\[/, "")
                    sub(/\]$/, "")
                    owned[$0] = 1
                }
                next
            }
            $2 ~ /:1F90$/ && $4 == "0A" && ($10 in owned) { found = 1 }
            END { exit !found }
        ' - "/proc/$server_pid/net/tcp" 2>/dev/null
}

ready=0
deadline=$(($(date +%s) + 10))
while child_running; do
    if [ "$(dd bs=1 count=1 iflag=nonblock status=none <&8 2>/dev/null || :)" = q ]; then
        exit "$((1 - ready))"
    fi
    if [ "$ready" -eq 0 ]; then
        [ "$(date +%s)" -lt "$deadline" ] || fail "Timed out waiting for webfsd to serve $url"
        if owns_listener &&
            curl --noproxy '*' --silent --fail --max-time 1 "$url" | cmp -s index.html - &&
            owns_listener; then
            printf '%s\n' 'DEV_SERVER_READY'
            ready=1
        fi
    fi
    sleep 0.1
done

status=0
wait "$server_pid" || status=$?
server_pid=
printf '%s\n' 'webfsd exited unexpectedly. Check whether port 8080 is already in use.' >&2
exit "$((status == 0 ? 1 : status))"
