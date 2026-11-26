# example_project

Example project, as a template for C17 projects that target the browser. The example is a click counter built with Clay. Clang and LLD compile and link it to freestanding WebAssembly without Emscripten or a C standard library. Javascript renders the UI in the browser.

The template includes native C tests, a Chromium end-to-end test, formatting, static and dynamic analysis, and code coverage. Develop on Linux locally or in a Docker or Podman dev container. Use GNU Make directly, run `sh scripts/dev.sh`, or use the configured VS Code tasks.


## Prerequisites

> Note that this setup assumes you are using Debian/Ubuntu (Linux).

- Container Engine (required for dev container development)
    - Docker:
        ```sh
        curl -fsSL https://get.docker.com -o get-docker.sh
        sudo sh ./get-docker.sh --dry-run
        ```
    - Podman:
        ```sh
        sudo apt update && sudo apt install -y podman
        ```
- VS Code, go to `https://code.visualstudio.com/docs/setup/linux` and follow the instructions (Linux specific).
    - VS Code Extensions:
        - Dev Containers `https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers`
            - NB! Make sure Dev containers plugin is pointing to Podman, if you are not using Docker. Check `$HOME/.config/Code/User/settings.json` (for Linux).
        - These already included inside the dev container:
            <!-- - C/C++ `https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools` -->
            - clangd `https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd`
            - EditorConfig `https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig`
            - C++ TestMate `https://marketplace.visualstudio.com/items?itemName=TestMate.vscode-catch2-test-adapter`
            - Coverage Gutters `https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters`
            - Live Server `https://marketplace.visualstudio.com/items?itemName=ritwickdey.liveserver`    
- Chromium: `sudo apt install chromium` (application specific, for debugging)

> Run commands from project root folder (e.g. `sh scripts/dev.sh <command>`). From another directory, use an absolute paths.


## Environment Setup

### Dev container

In VS Code, run **Dev Containers: Reopen in Container** and select either the Docker or Podman configuration.

The container is configured to do everything needed to prepare the project.

You can skip **Local setup without a dev container** from here.


### Local setup without a dev container

This is the "manual" approach. Install the development dependencies:

```sh
sudo apt-get update
sudo apt-get install -y \
    locales sudo git make pkg-config \
    clang lld llvm wabt clangd-19 bear clang-format-19 editorconfig \
    gdb cppcheck clang-tidy valgrind libclang-rt-dev \
    libgtest-dev libgmock-dev lcov \
    webfs media-types curl util-linux ca-certificates
sudo apt-get install -y nodejs npm
```

> NB! Make sure you have Node 20+ (`nodejs`)

Prepare the project after installing the dependencies:

```sh
sh scripts/dev.sh prepare
```

This initializes the Clay submodule and generates `compile_commands.json`.
In VS Code, run **Tasks: Run Task** → **Prepare dependencies and compilation database** to do the same.


#### Generate the compilation database

> NB! No need to run this during the setup, the `sh script/dev.sh prepare`-command is sufficient.

Run this whenever you make changes to compiler flags, include paths, or compiled source files. Ordinary source edits do not require regeneration.

```sh
sh scripts/dev.sh compdb
```

Bear uses wrapper mode to capture the Clang driver without also recording its internal `-cc1` invocation. It captures the release build and writes `compile_commands.json` to the repository root.


## Build & Run

Build `release` or `debug`:

```sh
sh scripts/dev.sh build release  # or just: sh scripts/dev.sh build
sh scripts/dev.sh build debug
```

| Mode | Compiler and linker settings | Output directory |
| --- | --- | --- |
| Release (default) | `-Os -Wall -Wextra`, with debug information stripped | `src/build/release/` |
| Debug | `-O0 -g -Wall -Wextra -Wpedantic -Werror`, with debug information preserved | `src/build/debug/` |


### Validate and inspect WebAssembly

Use WABT to validate or inspect the release WebAssembly module. Both commands build it first if needed:

```sh
sh scripts/dev.sh validate
sh scripts/dev.sh inspect
```

For instruction-level disassembly:

```sh
wasm-objdump -d src/build/release/index.wasm
```

### Serve (run) the application

(Build and) serve the selected mode with webfsd:

```sh
sh scripts/dev.sh serve release  # or just: sh scripts/dev.sh serve
sh scripts/dev.sh serve debug
```

Then open a web browser and check `http://127.0.0.1:8080/`

The server stays in the foreground; use **Ctrl+C** to stop it before switching modes.

In VS Code, choose **Tasks: Run Task** → **Serve (Debug)** to build and serve the debug app without launching the debugger. Use **Stop (Debug)** to stop it.

(TODO: Test Live Server!)

The optional **Live Server** extension is a separate server for `src/build/release/`, not a caller of `dev.sh`. Use it or the script's webfsd server, not both on port 8080.

## Stop & Clean

To stop the standalone server, use **Tasks: Run Task** → **Stop (Serve)** or run:
```sh
sh scripts/dev.sh stop
```

Prefer **Ctrl+C** in the serving terminal if other `webfsd` instances are running: when no valid PID file is found, the stop command falls back to scanning for `webfsd` processes. For the **Serve (Debug)** task, use **Stop (Debug)**.

Clean build artifacts:
```sh
sh scripts/dev.sh clean
```

## Debug (in VS Code)

> Before continuing make sure to have nothing running or stop anything using port 8080

In **Run and Debug**, select **Debug WebAssembly (Chrome)** and press **F5**

You will have a debug session with Chrome/Chromium.


## Testing and code quality

### Test suites

Start with the [native suite](#native-unit-and-integration-tests) for C logic and layout changes. Use the [browser suite](#browser-end-to-end-test) to check the built Wasm app and JavaScript renderer together.

#### Native unit and integration tests

```sh
sh scripts/dev.sh test native
```

The native suite runs both tests (unit test and integration test) in one executable, with its entry point in [`tests/native_main.c`](tests/native_main.c).

- [`tests/unit/`](tests/unit/) checks the counter logic in isolation with plain C assertions.
- [`tests/integration/`](tests/integration/) exercises the app's frame code with real Clay layout. Its text-measurement callback is a stub: it returns predictable dimensions without verifying calls.

#### Browser end-to-end test

Install the Node dependencies and Playwright Chromium once for both browser testing and coverage (available in the dev container):

```sh
npm ci --prefix tests
npm exec --prefix tests -- playwright-core install --with-deps --only-shell chromium
```

And run test:

```sh
sh scripts/dev.sh test browser
```

The scenario and its runner live in [`tests/end-to-end/`](tests/end-to-end/). The runner serves the release HTML and Wasm on a temporary loopback port and checks that pointer clicks update the displayed counter in headless Chromium. It exits without producing a coverage report.

If you already have a compatible Chromium and its system libraries, skip the browser install and set `CHROMIUM` to its executable path for `test browser` or `coverage`, for example:

```sh
CHROMIUM=/usr/bin/chromium sh scripts/dev.sh test browser
CHROMIUM=/usr/bin/chromium sh scripts/dev.sh coverage
```

You can use `command -v chromium` to find the executable path (Todo: figure out if you do this only in the container, or connect chromium on host side?)


### Dynamic analysis

These modes run the same native tests with runtime checks for memory errors and undefined behavior:

```sh
sh scripts/dev.sh test native valgrind
sh scripts/dev.sh test native sanitize
```

- **Valgrind Memcheck** runs the regular native test binary with debug symbols. Memory errors and definite or possible leaks fail the run; still-reachable allocations are reported without failing it.
- **Clang sanitizers** build a separate binary in `src/build/sanitize/` with AddressSanitizer and UndefinedBehaviorSanitizer. Leak detection is enabled on Linux. Findings fail the run, and `llvm-symbolizer` resolves stack frames to source locations.

These checks cover only the native C paths the tests execute, not the browser's WebAssembly or JavaScript renderer.

### Static analysis

Initialize the Clay submodule first; this command does not require a generated `compile_commands.json`.

```sh
sh scripts/dev.sh analyze
```

With no tool arguments, `analyze` runs all three tools: **Clang Static Analyzer** (`clang --analyze`), **clang-tidy** (bug-prone checks configured in `.clang-tidy`), and **cppcheck** (errors plus warning, performance, and portability checks). No executable is produced or run. Diagnostics selected by these checks fail the command.

To run only selected tools, pass one or more names: `clang` (Clang Static Analyzer), `clang-tidy`, or `cppcheck`. For example:

```sh
sh scripts/dev.sh analyze # Same as: sh scripts/dev.sh analyze clang clang-tidy cppcheck
sh scripts/dev.sh analyze clang
sh scripts/dev.sh analyze clang-tidy
sh scripts/dev.sh analyze cppcheck
sh scripts/dev.sh analyze clang clang-tidy
sh scripts/dev.sh analyze clang cppcheck
sh scripts/dev.sh analyze clang-tidy cppcheck
sh scripts/dev.sh analyze clang cppcheck
```

To continue with the other tools after a failure, run `make -k -C src analyze` (this tells `make` to keep going with the remaining tools).

The workflow checks the application C sources and native C tests. The Clang tools use the WebAssembly compiler flags for the application and native flags for the tests. Cppcheck uses its native platform model and excludes diagnostics located in the vendored Clay headers.

### Code coverage

```sh
sh scripts/dev.sh coverage
```

Coverage reruns the native and browser suites with instrumentation; it is not a separate suite. [`tests/coverage/`](tests/coverage/) contains the browser instrumentation and reporting script, which reuses the end-to-end scenario. Native coverage objects rebuild automatically if the Clang toolchain or checkout path changes.

| Coverage | Files | How it is measured |
| --- | --- | --- |
| Native C | `src/main.c`, `src/counter.c` | [Clang source-based coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html) from the native tests. |
| Browser JavaScript | Script in `src/web/index.html` | Istanbul coverage from the Chromium counter test, which sends real clicks and advances frames with Playwright's test clock. |

The browser test runs the real Wasm app, but the Wasm binary is not instrumented. Coverage excludes Clay, tests, build scripts, HTML markup, and CSS. There is no percentage gate.

#### View the combined report

- **Browser:** Open `src/build/coverage/html/index.html` for line and branch coverage
- **VS Code:** Run **Code Coverage**, then open the Command Palette (**Ctrl+Shift+P**) and select **Coverage Gutters: Watch**
- **Editors and CI:** Read `src/build/coverage/lcov.info`

### Formatting

```sh
sh scripts/dev.sh format       # rewrite tracked C sources and headers
sh scripts/dev.sh format-check # report formatting differences without editing
```

Both commands use clang-format 19 and the root `.clang-format`. They skip the Clay submodule, untracked files, and generated build outputs. Stage new C files to include them in the check. Use `format-check` in CI.

VS Code formats C on save through clangd 19. The EditorConfig extension applies the root `.editorconfig` whitespace rules for all supported file types. The optional `editorconfig` CLI reports the resolved rules for a path; it does not lint or format files.

## Command reference

These are equivalent ways to run the same workflows.

| Workflow | VS Code / Dev Container entrypoint | Portable script command | Equivalent underlying command |
| --- | --- | --- | --- |
| Prepare dependencies and compilation database | **Tasks: Run Task** → **Prepare (dependencies and compilation database)**; also runs automatically on every Docker or Podman dev container start | `sh scripts/dev.sh prepare` | `make -C src prepare` (initialize submodules, then run `compdb`) |
| Regenerate compilation database | **Tasks: Run Task** → **Generate compilation database** | `sh scripts/dev.sh compdb` | `make -C src compdb` |
| Validate WebAssembly | **Tasks: Run Task** → **Validate WebAssembly** | `sh scripts/dev.sh validate` | `make -C src validate` |
| Inspect WebAssembly | **Tasks: Run Task** → **Inspect WebAssembly** | `sh scripts/dev.sh inspect` | `make -C src inspect` |
| Build release | **Tasks: Run Build Task** → **Build (Release)** | `sh scripts/dev.sh build release` | `make -C src release` (also the default target) |
| Build debug | **Tasks: Run Task** → **Build (Debug)** | `sh scripts/dev.sh build debug` | `make -C src debug` |
| Rebuild release | **Tasks: Run Task** → **Rebuild (Release)** | `sh scripts/dev.sh rebuild release` | `make -C src rebuild-release` (also `make -C src rebuild`) |
| Rebuild debug | **Tasks: Run Task** → **Rebuild (Debug)** | `sh scripts/dev.sh rebuild debug` | `make -C src rebuild-debug` |
| Clean | **Tasks: Run Task** → **Clean** | `sh scripts/dev.sh clean` | `make -C src clean` |
| Build and serve release | **Tasks: Run Task** → **Run (Serve) (Release)** | `sh scripts/dev.sh serve release` | `make -C src serve MODE=release` (also `make -C src serve`) |
| Build and serve debug | **Tasks: Run Task** → **Run (Serve) (Debug)** | `sh scripts/dev.sh serve debug` | `make -C src serve MODE=debug` |
| Stop the standalone server | **Tasks: Run Task** → **Stop (Serve)** | `sh scripts/dev.sh stop` | `make -C src stop` |
| Run native tests | **Tasks: Run Test Task** → **Test (Native)** | `sh scripts/dev.sh test native` | `make -C src test-native` |
| Run native tests under Memcheck | **Tasks: Run Task** → **Test (Native: Valgrind)** | `sh scripts/dev.sh test native valgrind` | `make -C src test-native-valgrind` |
| Run native tests with Clang sanitizers | **Tasks: Run Task** → **Test (Native: Sanitizers)** | `sh scripts/dev.sh test native sanitize` | `make -C src test-native-sanitize` |
| Run browser test | **Tasks: Run Task** → **Test (Browser)** | `sh scripts/dev.sh test browser` | `make -C src test-browser` |
| Run all static analyzers | **Tasks: Run Task** → **Analyze (clang-tidy, Clang Static Analyzer, cppcheck)** | `sh scripts/dev.sh analyze` | `make -C src analyze` |
| Run Clang Static Analyzer only | **Tasks: Run Task** → **Analyze (Clang Static Analyzer)** | `sh scripts/dev.sh analyze clang` | `make -C src analyze-clang` |
| Run clang-tidy only | **Tasks: Run Task** → **Analyze (clang-tidy)** | `sh scripts/dev.sh analyze clang-tidy` | `make -C src analyze-clang-tidy` |
| Run cppcheck only | **Tasks: Run Task** → **Analyze (cppcheck)** | `sh scripts/dev.sh analyze cppcheck` | `make -C src analyze-cppcheck` |
| Run a combination of static analyzers | Pass tool names in the integrated terminal | (example) `sh scripts/dev.sh analyze clang-tidy cppcheck` | `make -C src analyze-clang-tidy analyze-cppcheck` |
| Generate code coverage | **Tasks: Run Task** → **Code Coverage** | `sh scripts/dev.sh coverage` | `make -C src coverage` |
| Format tracked C files | **Tasks: Run Task** → **Format (formats C files)** | `sh scripts/dev.sh format` | `clang-format-19 -i` on tracked `.c` and `.h` files |
| Check tracked C formatting | **Tasks: Run Task** → **Format (Checks C files)** | `sh scripts/dev.sh format-check` | `clang-format-19 --dry-run --Werror` on the same files |
| Show command help | No dedicated task; run the script in the integrated terminal | `sh scripts/dev.sh help` | Prints the script's usage information; no build command |


## Development Tools Overview

| Tool type | Tools | Purpose |
| --- | --- | --- |
| Development environment | Debian dev containers (Docker or Podman) | Provide a consistent development environment. |
| IDE | VS Code | Editor and entry point for development tasks and browser debugging. |
| Build system | GNU Make | Defines the build, serve, and test workflows in `src/Makefile`. |
| Compiler and linker | Clang and LLD | Compile and link the C application into WebAssembly. |
| Compilation database and language server | Bear and clangd | Bear records build commands in `compile_commands.json`; clangd uses them for editor completion, diagnostics, and navigation. |
| WebAssembly tooling | WABT | Validates and inspects WebAssembly; `wasm-objdump` also disassembles it. |
| Browser debugger | VS Code JavaScript debugger | Runs the configured browser debug session. |
| Native debugger | GDB | Available for native debugging; not wired into the VS Code launch configuration. |
| Unit and integration testing | C assertions | Verify behavior in the native unit and integration tests. |
| Browser end-to-end testing | Playwright and Chromium | Run the browser end-to-end test against the Wasm application. |
| Optional C++ test tooling | Google Test, Google Mock, and C++ TestMate | Available in the dev containers for projects that need a C++ test framework, interaction verification with mocks, or editor test discovery. The current suite uses none of these. |
| Dynamic analysis | Valgrind Memcheck | Checks native execution for memory errors and leaks. |
| Dynamic analysis | Clang ASan, LeakSanitizer, and UBSan | Instrument the native suite to detect memory errors, leaks on Linux, and undefined behavior. |
| Static analysis | Clang Static Analyzer, clang-tidy, and cppcheck | Check C source without running it. The project calls the analyzer through `clang --analyze`; the dev containers also provide `scan-build-19`. |
| Native code coverage | `llvm-profdata` and `llvm-cov` | Merge native coverage profiles, summarize coverage, and export an LCOV tracefile. |
| JavaScript code coverage | Istanbul | Instruments the browser's JavaScript renderer and exports an LCOV tracefile. |
| Coverage reporting and visualization | LCOV `genhtml` and Coverage Gutters | Render the combined coverage report as HTML and show coverage in VS Code, respectively. |
| Formatting and editor conventions | clang-format 19 and EditorConfig | Apply C formatting and editor whitespace conventions, respectively. |
| Static web server and MIME types | webfs and media-types | Serve the built application with the WebAssembly MIME type. |
| Static web server | Live Server | Optional VS Code alternative for serving the release build. |

## Pipeline

(TODO: Setup a pipeline with github actions or something similar? to have a working pipeline do all the testings)


## License

MIT License, see `LICENSE` for details.


## PLAN (TODOs)

| Task | Status |
| --- | --- |
| Implement clang | DONE |
| Implement GDB debugging | PARTLY |
| Implement Code formatter (clang-format, EditorConfig) | DONE |
| Implement project code coverage (Clang, Istanbul, LCOV, Coverage Gutters) | DONE |
| Implement native dynamic analysis (Valgrind, Clang ASan/UBSan) | DONE |
| Implement static analysis (Clang Static Analyzer, clang-tidy, cppcheck) | DONE |
| Implement native C unit and integration tests | DONE |
| Implement project specific stuff (live server, plus what else?) | PARTLY |
| List up all project specific stuff (e.g. live server, webfs and media-types, bear and clangd, WABT, ...?) | PARTLY |
| Map out options for pipelines, and Implement pipeline that does all the testing as well! (Github action? Azure DevOps Pipeline?)| |



