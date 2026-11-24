# pure_c_example_project — Clay web UI, compiled to freestanding wasm32.
#
#   make release build optimized WebAssembly in build/release/ (default)
#   make debug   build unoptimized WebAssembly with debug info in build/debug/
#   make prepare initialize submodules and regenerate the compilation database
#   make compdb regenerate the compilation database through a forced release build
#   make rebuild-release force a release rebuild (also make rebuild)
#   make rebuild-debug   force a debug rebuild
#   make test     run native C tests
#   make coverage run native C tests with coverage and generate LCOV and HTML reports
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
WASM_HEADERS  := clay/clay.h counter.h

.PHONY: all release debug prepare compdb rebuild rebuild-release rebuild-debug \
        test coverage validate inspect serve stop clean

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

test: $(TEST_BUILD)/counter_test
	$<

coverage: $(COVERAGE_BUILD)/counter_test
	rm -f "$(COVERAGE_BUILD)/counter.profraw" "$(COVERAGE_BUILD)/counter.profdata" "$(COVERAGE_BUILD)/lcov.info"
	rm -rf "$(COVERAGE_BUILD)/html"
	LLVM_PROFILE_FILE="$(abspath $(COVERAGE_BUILD))/counter.profraw" $<
	$(LLVM_PROFDATA) merge -sparse "$(COVERAGE_BUILD)/counter.profraw" -o "$(COVERAGE_BUILD)/counter.profdata"
	$(LLVM_COV) report -instr-profile="$(COVERAGE_BUILD)/counter.profdata" $<
	$(LLVM_COV) export -format=lcov -instr-profile="$(COVERAGE_BUILD)/counter.profdata" $< > "$(COVERAGE_BUILD)/lcov.info.tmp"
	mv "$(COVERAGE_BUILD)/lcov.info.tmp" "$(COVERAGE_BUILD)/lcov.info"
	genhtml --branch-coverage --output-directory "$(COVERAGE_BUILD)/html" "$(COVERAGE_BUILD)/lcov.info"

$(TEST_BUILD)/counter_test.o: ../tests/unit_test/counter_test.c counter.h Makefile | $(TEST_BUILD)
	$(CC) $(TEST_CFLAGS) -I. -c -o $@ $<

$(TEST_BUILD)/counter_test: counter.c $(TEST_BUILD)/counter_test.o counter.h Makefile
	$(CC) $(TEST_CFLAGS) -o $@ $< $(TEST_BUILD)/counter_test.o

$(COVERAGE_BUILD)/counter.o: counter.c counter.h Makefile | $(COVERAGE_BUILD)
	$(CC) $(TEST_CFLAGS) $(PROFILE_FLAG) -fcoverage-mapping -c -o $@ $<

$(COVERAGE_BUILD)/counter_test: $(COVERAGE_BUILD)/counter.o $(TEST_BUILD)/counter_test.o Makefile
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
