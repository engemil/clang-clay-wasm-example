#!/bin/sh
set -eu
# NB! Running this script with 'sh ...' means changes such as PATH exports disappear when it exits.

# Resolve repository paths independently of the caller's working directory.
root=$(CDPATH= cd "$(dirname "$0")/.." && pwd)
cd "$root"

# usage prints the usage message to stdout.
usage() {
    cat <<'EOF'
Usage: sh scripts/dev.sh {build|rebuild|serve} [release|debug]
       sh scripts/dev.sh {prepare|test|coverage|clean|stop|compdb|help}

Commands:
    prepare  Initialize submodules, then regenerate the compilation database.
    build    Build the WebAssembly application (release by default).
    rebuild  Force a rebuild of only the selected mode (release by default).
    test     Run the native C tests.
    coverage Run native and browser tests and generate LCOV and HTML reports.
    clean    Remove all build outputs.
    serve    Build and serve the selected mode (release by default; Ctrl+C to stop).
    stop     Stop the server using the Makefile's stop target.
    compdb   Regenerate compile_commands.json through a forced release build.
    help     Show this help.

Install the development dependencies listed in README.md before preparing.
Server URL: http://127.0.0.1:8080/ (available after the server starts).
EOF
}

# Main script entry point.
case "${1:-help}" in
    prepare|compdb|test|coverage)
        exec make -C "$root/src" "$1"
        ;;
    build|rebuild|serve)
        mode=${2-release}
        case "$mode" in
            release|debug) ;;
            *)
                printf 'Unknown build mode: %s\n' "$mode" >&2
                usage >&2
                exit 2
                ;;
        esac
        if [ "$#" -gt 2 ]; then
            usage >&2
            exit 2
        fi
        case "$1" in
            build)   exec make -C "$root/src" "$mode" ;;
            rebuild) exec make -C "$root/src" "rebuild-$mode" ;;
            serve)
                printf '%s\n' 'Server URL: http://127.0.0.1:8080/ (available after the server starts).'
                exec make -C "$root/src" serve "MODE=$mode"
                ;;
        esac
        ;;
    clean|stop)
        # Stop the server and remove build artifacts.
        exec make -C "$root/src" "$1"
        ;;
    help|--help|-h)
        # Show the usage message.
        usage
        ;;
    *)
        # Show the usage message and exit with an error code.
        printf 'Unknown command: %s\n' "$1" >&2
        usage >&2
        exit 2
        ;;
esac
