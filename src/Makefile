# pure_c_example_project — Clay web UI, compiled to freestanding wasm32.
#
#   make        build build/index.wasm + build/index.html
#   make serve  serve build/ on http://localhost:8080
#   make stop   stop the webfsd started by `make serve`
#   make clean  remove build artifacts
#
# Build depends on clay submodule: no emscripten, no libc, just clang + wasm-ld.

CC      := clang
BUILD   := build
PID     := $(shell cat $(BUILD)/webfsd.pid 2>/dev/null)
CFLAGS  := -Os -Wall -Wextra -DCLAY_WASM -mbulk-memory \
           --target=wasm32 -nostdlib \
           -Wl,--strip-all -Wl,--export-dynamic -Wl,--no-entry \
           -Wl,--export=__heap_base -Wl,--initial-memory=6553600

.PHONY: all serve stop clean

all: $(BUILD)/index.wasm $(BUILD)/index.html

$(BUILD)/index.wasm: main.c clay/clay.h | $(BUILD)
	$(CC) $(CFLAGS) -Iclay -o $@ main.c
$(BUILD)/index.html: web/index.html | $(BUILD)
	cp $< $@


$(BUILD):
	mkdir -p $@

# Lightweight C static server (webfsd); .wasm served as application/wasm via /etc/mime.types.
serve: all
	cd $(BUILD) && rm -f webfsd.pid && exec webfsd -F -p 8080 -f index.html -k webfsd.pid

# Stops the webfsd started by `make serve`
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
	@rm -f $(BUILD)/webfsd.pid

clean:
	rm -rf $(BUILD)
