# example_project

example project (acting as a template)


## Prerequisites

> Note that this setup assumes you are using Debian/Ubuntu and will use a container engine and VS Code with dev container. However, it is sat up in a way to be possible to run from terminal with sh-script(s).

- Container Engine (required only for dev-container development)
    - Docker, preview the installation script (`--dry-run` does not install Docker):
        ```sh
        curl -fsSL https://get.docker.com -o get-docker.sh
        sudo sh ./get-docker.sh --dry-run
        ```
    - Podman, :
        ```sh
        sudo apt update && sudo apt install -y podman
        ```
- VS Code, go to https://code.visualstudio.com/docs/setup/linux
    - VS Code Extensions:
        <!-- - C/C++ https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools -->
        - clangd https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd
        - EditorConfig https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig
        - C++ TestMate https://marketplace.visualstudio.com/items?itemName=TestMate.vscode-catch2-test-adapter
        - gcov Viewer https://marketplace.visualstudio.com/items?itemName=JacquesLucke.gcov-viewer
        - Live Server https://marketplace.visualstudio.com/items?itemName=ritwickdey.liveserver
        - (Optional) Dev Containers https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers
            - NB! Make sure Dev containers plugin is pointing to Podman, if you are not using Docker. Check `$HOME/.config/Code/User/settings.json` (for Linux).


> Run commands from entry point (e.g. `sh scripts/dev.sh <command>`). From another directory, use an absolute paths.


## Environment Setup

### Dev container

In VS Code, run **Dev Containers: Reopen in Container** and select either the Docker or Podman configuration.

The container is configured to do everything needed to prepare the project.

You can skip **Local setup without a dev container** from here.


### Local setup without a dev container

If you prepare to not run in container, this is the "manual" approach.

Install the development dependencies:

(TODO: also go over to see if we are installing too much uncessary or double installing in Dockerfile(s))

```sh
sudo apt-get update
sudo apt-get install -y \
    locales sudo git build-essential pkg-config \
    clang-tools clang lld clangd bear clang-format editorconfig \
    gdb cppcheck clang-tidy valgrind libclang-rt-dev \
    libgtest-dev libgmock-dev lcov \
    webfs media-types curl ca-certificates
```

Prepare the project after installing the dependencies:

```sh
sh scripts/dev.sh prepare
```

This initializes the Clay submodule and generates `compile_commands.json`.


#### Generate the compilation database

No need to run this during the setup `prepare` is sufficient.

> NB! Rerun `compdb` after changing compiler flags, include paths, or compiled source files. Ordinary source edits do not require regeneration.

```sh
sh scripts/dev.sh compdb
```


## Build & Run

Build:
```sh
sh scripts/dev.sh build
```

Run (start webfsd):
```sh
sh scripts/dev.sh serve
```

Then open a web browser and check `http://127.0.0.1:8080/`

The server stays in the foreground; use Ctrl+C to stop it.

The optional **Live Server** extension is a separate server for `src/build/`, not a caller of `dev.sh`; use it or the script's webfsd server, not both on port 8080.

## Stop & Clean

Stop webfsd:
```sh
sh scripts/dev.sh stop
```

Clean build artifacts:
```sh
sh scripts/dev.sh clean
```

## Command reference

These are equivalent ways to run the same workflows.

| Workflow | VS Code / Dev Container entrypoint | Portable script command | Equivalent underlying command |
| --- | --- | --- | --- |
| Prepare dependencies and compilation database | Automatic `postStartCommand` on every dev container start, for both Docker and Podman | `sh scripts/dev.sh prepare` | `git submodule update --init --recursive && bear --output compile_commands.json -- make -C src -B` |
| Build | **Tasks: Run Build Task** → **Build** (default build task) | `sh scripts/dev.sh build` | `make -C src` |
| Clean | **Tasks: Run Task** → **Clean** | `sh scripts/dev.sh clean` | `make -C src clean` |
| Build and serve | **Tasks: Run Task** → **Run (Serve)**; also the default task for **Tasks: Run Test Task** | `sh scripts/dev.sh serve` | `make -C src serve` |
| Regenerate compilation database | **Tasks: Run Task** → **Generate compilation database** | `sh scripts/dev.sh compdb` | `bear --output compile_commands.json -- make -C src -B` |
| Stop the server | No dedicated task; run the script in the integrated terminal, or use Ctrl+C in the serving terminal | `sh scripts/dev.sh stop` | `make -C src stop` |
| Show command help | No dedicated task; run the script in the integrated terminal | `sh scripts/dev.sh help` | Prints the script's usage information; no build command |

## Development Tools Overview

(TODO: Improve!)

| Tools | Purpose |
| --- | --- |
| Clang and LLD | Compile and link the C application into WebAssembly through `src/Makefile`. |
| Bear and clangd | Record the build commands in `compile_commands.json` and use them for editor completion, diagnostics, and navigation. |
| clang-format | Format C/C++ using the repository's `.clang-format` rules. VS Code can format through the clangd extension. |
| EditorConfig | Apply `.editorconfig` whitespace conventions through the editor extension. The CLI package alone does not configure VS Code. |
| clang-tidy, cppcheck, and clang-tools | Provide static analysis and additional Clang utilities. Installation alone does not run analysis. |
| GDB | Debug native executables; it is not configured here to debug the browser's WebAssembly execution. |
| Valgrind and Clang runtime libraries | Support native dynamic analysis and sanitizer builds. Valgrind does not directly analyze the `.wasm` output, and sanitizers require an appropriate instrumented build. |
| Google Test and Google Mock | Provide C++ testing and mocking libraries for separately configured tests. |
| C++ TestMate | Discover and run supported C++ test executables in VS Code once those executables are built and configured. |
| LCOV and gcov Viewer | Collect/report coverage data and display gcov-format results in VS Code, respectively. An instrumented build and coverage-generation commands are required. |
| webfs and media-types | Serve the built application with `sh scripts/dev.sh serve`, including the WebAssembly MIME type. |
| Live Server | Provide an optional VS Code web server, configured to serve `src/build/`. Use it or webfs, not both on port 8080. |

The current Makefile builds and serves the WebAssembly application. It does not
define test, static-analysis, sanitizer, or coverage targets. Installing the tools
and extensions does not enable those workflows automatically.


## Pipeline

(TODO: Setup a pipeline with github actions or something similar? to have a working pipeline do all the testings)


## License

MIT License, see `LICENSE` for details.
