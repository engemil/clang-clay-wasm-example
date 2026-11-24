# example_project

example project (acting as a template)

(TODO: add to the point info about this example setup. I want it to show specified dev, e.g. clang (not gcc), etc...) Builds use C17 and freestanding compilation.



## Prerequisites

> Note that this setup assumes you are using Debian/Ubuntu (Linux) and will run dev container. Follow the instructions under **Local setup without a dev container** under **Environment Setup** if you are not to use VS code and/or container(s).

- Container Engine (required only for dev-container development)
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
    - VS Code Extensions (already included inside the dev container):
        <!-- - C/C++ `https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools` -->
        - clangd `https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd`
        - EditorConfig `https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig`
        - C++ TestMate `https://marketplace.visualstudio.com/items?itemName=TestMate.vscode-catch2-test-adapter`
        - Coverage Gutters `https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters`
        - Live Server `https://marketplace.visualstudio.com/items?itemName=ritwickdey.liveserver`
        - Dev Containers `https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers`
            - NB! Make sure Dev containers plugin is pointing to Podman, if you are not using Docker. Check `$HOME/.config/Code/User/settings.json` (for Linux).
- Chromium: `sudo apt install chromium` (application specific, for debugging)

> Run commands from entry point (e.g. `sh scripts/dev.sh <command>`). From another directory, use an absolute paths.


## Environment Setup

### Dev container

In VS Code, run **Dev Containers: Reopen in Container** and select either the Docker or Podman configuration.

The container is configured to do everything needed to prepare the project.

You can skip **Local setup without a dev container** from here.


### Local setup without a dev container

If you prepare to not run in container, this is the "manual" approach.

Install the development dependencies:

```sh
sudo apt-get update
sudo apt-get install -y \
    locales sudo git make pkg-config \
    clang lld llvm wabt clangd bear clang-format editorconfig \
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

In VS Code, **Tasks: Run Build Task** runs **Build (Release)** by default. For debug, use **Tasks: Run Task** → **Build (Debug)**.


### Validate and inspect WebAssembly

WABT provides optional validation and inspection targets:

```sh
make -C src validate
make -C src inspect
```

For instruction-level disassembly:

```sh
wasm-objdump -d src/build/release/index.wasm
```

### Serve (run) the application

Build and serve the selected mode with webfsd:

```sh
sh scripts/dev.sh serve release  # or just: sh scripts/dev.sh serve
sh scripts/dev.sh serve debug
```

Then open a web browser and check `http://127.0.0.1:8080/`

The server stays in the foreground; use **Ctrl+C** to stop it before switching modes.

(TODO: Test Live Server!)

The optional **Live Server** extension is a separate server for `src/build/release/`, not a caller of `dev.sh`. Use it or the script's webfsd server, not both on port 8080.

## Stop & Clean

Stop the serve:
```sh
sh scripts/dev.sh stop
```

Or just use **Ctrl+C** for the running foreground terminal.

Clean build artifacts:
```sh
sh scripts/dev.sh clean
```

## Debug (in VS Code)

> Before continuing make sure to have nothing running or stop anything using port 8080

In **Run and Debug**, select **Debug WebAssembly (Chrome)** and press **F5**

You will have a debug session with Chrome/Chromium.

## Tests and project coverage

`tests/` groups test related code.

### What coverage measures

| Coverage | Files | How it is measured |
| --- | --- | --- |
| Native C | `src/main.c`, `src/counter.c` | [Clang source-based coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html) from the native tests. |
| Browser JavaScript | Script in `src/web/index.html` | Istanbul coverage from the Chromium counter test, which sends real clicks and advances frames with Playwright's test clock. |

The browser test runs the real WASM app, but the WASM binary is not instrumented. Coverage excludes Clay, tests, build scripts, HTML markup, and CSS. There is no percentage gate.

### Native tests

```sh
sh scripts/dev.sh test
```

Runs the counter and frame-rendering tests (C code) without a browser.

### Set up and run coverage

`sh scripts/dev.sh coverage` runs the native C tests and the end-to-end Chromium test, then combines their C and JavaScript coverage into one report.

Run these commands:

```sh
npm ci --prefix tests
npm exec --prefix tests -- playwright-core install --with-deps --only-shell chromium
sh scripts/dev.sh coverage
```

Already have a compatible Chromium? Skip the browser install and set `CHROMIUM` to its executable path when running coverage. Native coverage objects rebuild automatically if the Clang toolchain or checkout path changes.

### View the combined report

- **Browser:** Open `src/build/coverage/html/index.html` for line and branch coverage
- **VS Code:** Run **Coverage (Project)**, then open the Command Palette (**Ctrl+Shift+P**) and select **Coverage Gutters: Watch**
- **Editors and CI:** Read `src/build/coverage/lcov.info`

## Command reference

These are equivalent ways to run the same workflows.

| Workflow | VS Code / Dev Container entrypoint | Portable script command | Equivalent underlying command |
| --- | --- | --- | --- |
| Prepare dependencies and compilation database | Automatic `postStartCommand` on every dev container start, for both Docker and Podman | `sh scripts/dev.sh prepare` | `make -C src prepare` (initialize submodules, then run `compdb`) |
| Build release | **Tasks: Run Build Task** (default: **Build (Release)**) | `sh scripts/dev.sh build release` | `make -C src release` (also the default target) |
| Build debug | **Tasks: Run Task** → **Build (Debug)** | `sh scripts/dev.sh build debug` | `make -C src debug` |
| Rebuild release | **Tasks: Run Task** → **Rebuild (Release)** | `sh scripts/dev.sh rebuild release` | `make -C src rebuild-release` (also `make -C src rebuild`) |
| Rebuild debug | **Tasks: Run Task** → **Rebuild (Debug)** | `sh scripts/dev.sh rebuild debug` | `make -C src rebuild-debug` |
| Clean | **Tasks: Run Task** → **Clean** | `sh scripts/dev.sh clean` | `make -C src clean` |
| Build and serve release | **Tasks: Run Task** → **Run (Serve)** | `sh scripts/dev.sh serve release` | `make -C src serve MODE=release` (also `make -C src serve`) |
| Build and serve debug | No dedicated task; run the script in the integrated terminal | `sh scripts/dev.sh serve debug` | `make -C src serve MODE=debug` |
| Run native tests | **Tasks: Run Test Task** (default: **Test (Native)**) | `sh scripts/dev.sh test` | `make -C src test` |
| Generate project coverage | **Tasks: Run Task** → **Coverage (Project)** | `sh scripts/dev.sh coverage` | `make -C src coverage` |
| Regenerate compilation database | **Tasks: Run Task** → **Generate compilation database** | `sh scripts/dev.sh compdb` | `make -C src compdb` |
| Stop the server | No dedicated task; run the script in the integrated terminal, or use Ctrl+C in the serving terminal | `sh scripts/dev.sh stop` | `make -C src stop` |
| Show command help | No dedicated task; run the script in the integrated terminal | `sh scripts/dev.sh help` | Prints the script's usage information; no build command |
| Validate WebAssembly | No dedicated task; run Make in the integrated terminal | — | `make -C src validate` |
| Inspect WebAssembly | No dedicated task; run Make in the integrated terminal | — | `make -C src inspect` |

## Development Tools Overview

(TODO: Improve!)

| Tools | Purpose |
| --- | --- |
| Clang and LLD | Compile and link the C application into WebAssembly through `src/Makefile`. |
| WABT | Validate the built module with `make -C src validate`, inspect it with `make -C src inspect`, or disassemble it with `wasm-objdump -d`. |
| Bear and clangd | Record the build commands in `compile_commands.json` and use them for editor completion, diagnostics, and navigation. |
| clang-format | Format C/C++ using the repository's `.clang-format` rules. VS Code can format through the clangd extension. |
| EditorConfig | Apply `.editorconfig` whitespace conventions through the editor extension. The CLI package alone does not configure VS Code. |
| clang-tidy, cppcheck, and clang-tools | Provide static analysis and additional Clang utilities. Installation alone does not run analysis. |
| GDB | Debug native executables; it is not configured here to debug the browser's WebAssembly execution. |
| Valgrind and Clang runtime libraries | Support native dynamic analysis and sanitizer builds. Valgrind does not directly analyze the `.wasm` output, and sanitizers require an appropriate instrumented build. |
| Google Test and Google Mock | Provide C++ testing and mocking libraries for separately configured tests. |
| C++ TestMate | Discover and run supported C++ test executables in VS Code once those executables are built and configured. |
| LLVM coverage tools | `llvm-profdata` indexes the raw Clang profile; `llvm-cov` prints the summary and exports the LCOV tracefile. |
| Playwright and Istanbul | Run the WASM app in headless Chromium and collect coverage for its JavaScript renderer. |
| LCOV and Coverage Gutters | LCOV's `genhtml` renders the tracefile as HTML; Coverage Gutters displays its line coverage in VS Code. |
| webfs and media-types | Serve the built application with `sh scripts/dev.sh serve`, including the WebAssembly MIME type. |
| Live Server | Provide an optional VS Code web server, configured to serve `src/build/release/`. Use it or webfs, not both on port 8080. |


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
| Implement Runtime analysis (llvm/clang sanitizers, valgrind) | PARTLY |
| Implement static analysis (clang static analyzer, clang-tidy, cppcheck) | PARTLY |
| Implement testing (unit test + mock) (Google Test + Google Mock, C++ TestMate) | PARTLY |
| Implement project specific stuff (live server, plus what else?) | PARTLY |
| List up all project specific stuff (e.g. live server, webfs and media-types, bear and clangd, WABT, ...?) | DONE |
| Map out options for pipelines, and Implement pipeline that does all the testing as well! (Github action? Azure DevOps Pipeline?)| |



