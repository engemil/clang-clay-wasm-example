# example_project

example project (acting as a template)


## Prerequisites

> Note that this setup assumes you are using Debian/Ubuntu

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


## Environment Setup

> NB! Run commands from the repository root unless stated otherwise.

### Dev container

In VS Code, run **Dev Containers: Reopen in Container** and select either the Docker or Podman configuration.

The container is configured to do everything needed to prepare the project. 

### Local setup without a dev container

Initialize the Clay dependency before building or opening the dev container:

```sh
git submodule update --init --recursive
```

Install the development dependencies:

```sh
sudo apt-get update
sudo apt-get install -y \
    locales sudo git build-essential pkg-config \
    clang-tools clang lld clangd bear clang-format editorconfig \
    gdb cppcheck clang-tidy valgrind libclang-rt-dev \
    libgtest-dev libgmock-dev lcov \
    webfs media-types curl ca-certificates
```

And if not already done (in prerequisits), install the recommended VS Code extensions.


#### Generate the compilation database

Run:

```sh
bear --output compile_commands.json -- make -C src -B
```

> NB! Remember to rerun the Bear command after changing compiler flags, include paths, or compiled source files. Ordinary source edits do not require regeneration.


## Build & Run

Run these commands from the repository root.

Build:
```sh
make -C src
```

Run (start webfsd):
```sh
make -C src serve
```

Then open a web browser and check `http://127.0.0.1:8080/`


## Stop & Clean

Stop webfsd:
```sh
make -C src stop
```

Clean build artifacts:
```sh
make -C src clean
```

## Development Tools Overview

(TODO: Improve!)
(TODO: also go over to see if we are installing too much uncessary or double installing in Dockerfile(s))

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
| webfs and media-types | Serve the built application with `make -C src serve`, including the WebAssembly MIME type. |
| Live Server | Provide an optional VS Code web server, configured to serve `src/build/`. Use it or webfs, not both on port 8080. |

The current Makefile builds and serves the WebAssembly application. It does not
define test, static-analysis, sanitizer, or coverage targets. Installing the tools
and extensions does not enable those workflows automatically.


## Pipeline

(TODO: Setup a pipeline with github actions or something similar? to have a working pipeline do all the testings)


## License

MIT License, see `LICENSE` for details.
