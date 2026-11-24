# pure_c_example_project — Clay web UI, compiled to freestanding wasm32.
#
#   make release build optimized WebAssembly in build/release/ (default)
#   make debug   build unoptimized WebAssembly with debug info in build/debug/
#   make prepare initialize submodules and regenerate the compilation database
#   make compdb regenerate the compilation database through a forced release build
#   make rebuild-release force a release rebuild (also make rebuild)
#   make rebuild-debug   force a debug rebuild
#   make test     run native C tests
#   make coverage run native and browser tests and generate LCOV and HTML reports
#   make validate validate the built WebAssembly module with WABT
#   make inspect inspect the built WebAssembly module with WABT
#   make serve [MODE=release|debug] build and serve the selected mode (default: release)
#   make stop   stop the webfsd started by make serve
#   make clean  remove build artifacts
#
# Build depends on clay submodule: no emscripten, no libc, just clang + wasm-ld.

CC            := clang
ROOT          := $(abspath ..)
BUILD         := build
RELEASE_BUILD := $(BUILD)/release
DEBUG_BUILD   := $(BUILD)/debug
TEST_BUILD    := $(BUILD)/test
COVERAGE_BUILD := $(BUILD)/coverage
MODE          ?= release
PID_FILE      := $(abspath $(BUILD)/webfsd.pid)
PID           := $(shell cat "$(PID_FILE)" 2>/dev/null)

COMMON_CFLAGS := -std=c17 -ffreestanding \
                 -Wall -Wextra -DCLAY_WASM -mbulk-memory \
                 --target=wasm32 -nostdlib \
                 -Wl,--export-dynamic -Wl,--no-entry \
                 -Wl,--export=__heap_base -Wl,--initial-memory=6553600
CFLAGS        := $(COMMON_CFLAGS) -Os -Wl,--strip-all
DEBUG_CFLAGS  := $(COMMON_CFLAGS) -O0 -g -Wpedantic -Werror

TEST_CFLAGS    := -std=c17 -Wall -Wextra -Wpedantic -Werror -g
PROFILE_FLAG   := -fprofile-instr-generate
LLVM_PROFDATA  ?= $(shell $(CC) -print-prog-name=llvm-profdata)
LLVM_COV       ?= $(shell $(CC) -print-prog-name=llvm-cov)
WASM_SOURCES  := main.c counter.c
WASM_HEADERS  := clay/clay.h counter.h frame.h
NATIVE_TOOLCHAIN := $(TEST_BUILD)/toolchain
TEST_OBJECTS  := $(addprefix $(TEST_BUILD)/,native_tests.o counter_test.o frame_test.o)
COVERAGE_OBJECTS := $(WASM_SOURCES:%.c=$(COVERAGE_BUILD)/%.o)
COVERAGE_FILTER := -ignore-filename-regex=/clay/

.PHONY: all release debug prepare compdb rebuild rebuild-release rebuild-debug \
        test coverage validate inspect serve stop clean check-native-toolchain

all: release

release: $(RELEASE_BUILD)/index.wasm $(RELEASE_BUILD)/index.html

debug: $(DEBUG_BUILD)/index.wasm $(DEBUG_BUILD)/index.html

prepare:
	git -C "$(ROOT)" submodule update --init --recursive
	$(MAKE) compdb

compdb:
	bear --force-wrapper --output "$(ROOT)/compile_commands.json" -- $(MAKE) -B all

rebuild: rebuild-release

rebuild-release:
	$(MAKE) -B release

rebuild-debug:
	$(MAKE) -B debug

test: $(TEST_BUILD)/native_tests
	$<

# Istanbul can mark function declarations that have no executable line entry.
coverage: $(COVERAGE_BUILD)/native_tests $(RELEASE_BUILD)/index.wasm
	rm -f "$(COVERAGE_BUILD)/native.profraw" "$(COVERAGE_BUILD)/native.profdata" "$(COVERAGE_BUILD)/lcov.info"
	rm -rf "$(COVERAGE_BUILD)/html"
	LLVM_PROFILE_FILE="$(abspath $(COVERAGE_BUILD))/native.profraw" $<
	$(LLVM_PROFDATA) merge -sparse "$(COVERAGE_BUILD)/native.profraw" -o "$(COVERAGE_BUILD)/native.profdata"
	$(LLVM_COV) report $(COVERAGE_FILTER) -instr-profile="$(COVERAGE_BUILD)/native.profdata" $<
	$(LLVM_COV) export -format=lcov $(COVERAGE_FILTER) -instr-profile="$(COVERAGE_BUILD)/native.profdata" $< > "$(COVERAGE_BUILD)/native.lcov"
	node "$(ROOT)/tests/coverage/browser_coverage.mjs" "$(RELEASE_BUILD)" "$(COVERAGE_BUILD)"
	cat "$(COVERAGE_BUILD)/native.lcov" "$(COVERAGE_BUILD)/browser.lcov" > "$(COVERAGE_BUILD)/lcov.info.tmp"
	mv "$(COVERAGE_BUILD)/lcov.info.tmp" "$(COVERAGE_BUILD)/lcov.info"
	genhtml --no-function-coverage --branch-coverage --source-directory "$(ROOT)" \
	    --output-directory "$(COVERAGE_BUILD)/html" "$(COVERAGE_BUILD)/lcov.info"

# A bind-mounted checkout can reuse build/ across compilers and source paths.
$(NATIVE_TOOLCHAIN): check-native-toolchain | $(TEST_BUILD)
	@set -e; { printf '%s\n' "$(CURDIR)" "$(CC)"; $(CC) --version; } > "$@.tmp"; \
	    if cmp -s "$@.tmp" "$@"; then rm -f "$@.tmp"; else mv -f "$@.tmp" "$@"; fi

$(TEST_BUILD)/native_tests.o: ../tests/shared/native_tests.c Makefile $(NATIVE_TOOLCHAIN) | $(TEST_BUILD)
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

$(TEST_BUILD)/counter_test.o: ../tests/unit/counter_test.c counter.h Makefile $(NATIVE_TOOLCHAIN) | $(TEST_BUILD)
	$(CC) $(TEST_CFLAGS) -I. -c -o $@ $<

$(TEST_BUILD)/frame_test.o: ../tests/integration/frame_test.c $(WASM_HEADERS) Makefile $(NATIVE_TOOLCHAIN) | $(TEST_BUILD)
	$(CC) $(TEST_CFLAGS) -I. -Iclay -c -o $@ $<

$(TEST_BUILD)/native_tests: $(WASM_SOURCES) $(TEST_OBJECTS) $(WASM_HEADERS) Makefile
	$(CC) $(TEST_CFLAGS) -Iclay -o $@ $(filter %.c %.o,$^)

$(COVERAGE_OBJECTS): $(COVERAGE_BUILD)/%.o: %.c $(WASM_HEADERS) Makefile $(NATIVE_TOOLCHAIN) | $(COVERAGE_BUILD)
	$(CC) $(TEST_CFLAGS) $(PROFILE_FLAG) -fcoverage-mapping -Iclay -c -o $@ $<

$(COVERAGE_BUILD)/native_tests: $(COVERAGE_OBJECTS) $(TEST_OBJECTS) Makefile
	$(CC) $(PROFILE_FLAG) -o $@ $(filter %.o,$^)

validate: $(RELEASE_BUILD)/index.wasm
	wasm-validate $<

inspect: $(RELEASE_BUILD)/index.wasm
	wasm-objdump -x $<

$(RELEASE_BUILD)/index.wasm: $(WASM_SOURCES) $(WASM_HEADERS) Makefile | $(RELEASE_BUILD)
	$(CC) $(CFLAGS) -Iclay -o $@ $(WASM_SOURCES)

$(DEBUG_BUILD)/index.wasm: $(WASM_SOURCES) $(WASM_HEADERS) Makefile | $(DEBUG_BUILD)
	$(CC) $(DEBUG_CFLAGS) -Iclay -o $@ $(WASM_SOURCES)

$(RELEASE_BUILD)/index.html: web/index.html | $(RELEASE_BUILD)
	cp $< $@

$(DEBUG_BUILD)/index.html: web/index.html | $(DEBUG_BUILD)
	cp $< $@

$(RELEASE_BUILD) $(DEBUG_BUILD) $(TEST_BUILD) $(COVERAGE_BUILD):
	mkdir -p $@

# Lightweight C static server (webfsd); .wasm served as application/wasm via /etc/mime.types.
WEBFSD            := webfsd -F -p 8080 -f index.html
SERVE_DIR_release := $(RELEASE_BUILD)
SERVE_DIR_debug   := $(DEBUG_BUILD)
SERVE_DIR         := $(SERVE_DIR_$(MODE))

serve: $(if $(SERVE_DIR),$(MODE))
	$(if $(SERVE_DIR),,$(error Unknown serve mode '$(MODE)'; expected release or debug))
ifeq ($(SERVE_WAIT),1)
	cd "$(SERVE_DIR)" && exec sh "$(ROOT)/scripts/wait-for-webfsd.sh" serve $(WEBFSD) -4
else
	cd "$(SERVE_DIR)" && rm -f "$(PID_FILE)" && exec $(WEBFSD) -k "$(PID_FILE)"
endif

# Stops the webfsd started by make serve.
stop:
	@if [ -n "$(PID)" ] && [ "$$(cat /proc/$(PID)/comm 2>/dev/null)" = webfsd ]; then \
		echo "stopping webfsd (pid $(PID))"; \
		kill $(PID); \
	else \
		found=0; \
		for f in /proc/[0-9]*/comm; do \
			read c < "$$f" 2>/dev/null || continue; \
			[ "$$c" = webfsd ] || continue; \
			p=$${f#/proc/}; p=$${p%/comm}; \
			echo "stopping webfsd (pid $$p, via /proc scan)"; \
			kill $$p; \
			found=1; \
		done; \
		if [ $$found -eq 0 ]; then echo "webfsd not running"; fi; \
	fi
	@rm -f "$(PID_FILE)"

clean:
	rm -rf $(BUILD)
