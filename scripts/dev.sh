#!/bin/sh
set -eu
# NB! Running this script with 'sh ...' means changes such as PATH exports disappear when it exits.

# Resolve repository paths independently of the caller's working directory.
root=$(CDPATH= cd "$(dirname "$0")/.." && pwd)
cd "$root"

# usage prints the usage message to stdout.
usage() {
    cat <<'EOF'
Usage: sh scripts/dev.sh {prepare|build|rebuild|clean|serve|stop|compdb|help}

Commands:
    prepare  Initialize submodules, then regenerate the compilation database.
    build    Build the WebAssembly application.
    rebuild  Remove build artifacts, then build the WebAssembly application.
    clean    Remove build artifacts.
    serve    Build and run the server in the foreground (Ctrl+C to stop).
    stop     Stop the server using the Makefile's stop target.
    compdb   Regenerate compile_commands.json through a forced build.
    help     Show this help.

Install the development dependencies listed in README.md before preparing.
Server URL: http://127.0.0.1:8080/ (available after the server starts).
EOF
}

# compdb generates the compilation database using Bear.
compdb() {
    exec bear --output "$root/compile_commands.json" -- \
        make -C "$root/src" -B
}

# Main script entry point.
case "${1:-help}" in
    prepare)
        # Initialize submodules and regenerate the compilation database.
        git submodule update --init --recursive
        compdb
        ;;
    compdb)
        # Regenerate the compilation database.
        compdb
        ;;
    build)
        # Build the WebAssembly application.
        exec make -C "$root/src"
        ;;
    rebuild)
        exec make -C "$root/src" rebuild
        ;;
    serve)
        # Build and run the server in the foreground.
        printf '%s\n' 'Server URL: http://127.0.0.1:8080/ (available after the server starts).'
        exec make -C "$root/src" serve
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
