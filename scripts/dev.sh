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
       sh scripts/dev.sh test native [valgrind|sanitize]
       sh scripts/dev.sh test browser
       sh scripts/dev.sh {prepare|coverage|analyze|clean|stop|compdb|help}
       sh scripts/dev.sh {format|format-check}

Commands:
    prepare  Initialize submodules, then regenerate the compilation database.
    build    Build the WebAssembly application (release by default).
    rebuild  Force a rebuild of only the selected mode (release by default).
    test native           Run the native C tests.
    test native valgrind  Run the native C tests under Valgrind Memcheck.
    test native sanitize  Run the native C tests with Clang ASan and UBSan.
    test browser          Run the Chromium test against the release build.
    coverage Run native and browser tests and generate LCOV and HTML reports.
    analyze  Run Clang Static Analyzer, clang-tidy, and cppcheck.
    clean    Remove all build outputs.
    serve    Build and serve the selected mode (release by default; Ctrl+C to stop).
    stop     Stop the server using the Makefile's stop target.
    compdb   Regenerate compile_commands.json through a forced release build.
    format   Format tracked C sources and headers in place.
    format-check  Fail if tracked C sources or headers need formatting.
    help     Show this help.

Install the development dependencies listed in README.md before preparing.
Server URL: http://127.0.0.1:8080/ (available after the server starts).
EOF
}

# Main script entry point.
case "${1:-help}" in
    prepare|compdb|coverage|analyze)
        exec make -C "$root/src" "$1"
        ;;
    test)
        shift
        case "$#:$*" in
            '1:native')          target=test-native ;;
            '2:native valgrind') target=test-native-valgrind ;;
            '2:native sanitize') target=test-native-sanitize ;;
            '1:browser')         target=test-browser ;;
            *)
                printf '%s\n' 'Expected: test native [valgrind|sanitize] or test browser.' >&2
                usage >&2
                exit 2
                ;;
        esac
        exec make -C "$root/src" "$target"
        ;;
    format|format-check)
        if ! command -v clang-format-19 >/dev/null 2>&1; then
            printf '%s\n' 'clang-format-19 is required; see README.md.' >&2
            exit 127
        fi
        case "$1" in
            format)       set -- -i ;;
            format-check) set -- --dry-run --Werror ;;
        esac
        # Keep Git's exit status and pass filenames without shell splitting.
        files=$(mktemp)
        trap 'rm -f "$files"' EXIT
        git ls-files -z -- '*.c' '*.h' > "$files"
        xargs -0 -r clang-format-19 "$@" -- < "$files"
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
