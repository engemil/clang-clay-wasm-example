# example_project

example project (acting as a template)


## Prerequisites

> Note that this setup assumes you are using Debian/Ubuntu

- Container Engine
    - Docker, easy script to run:
        ```sh
        curl -fsSL https://get.docker.com -o get-docker.sh
        sudo sh ./get-docker.sh --dry-run
        ```
    - Podman, :
        ```sh
        sudo apt update && sudo apt install -y podman
        ```
- (optional) VS Code, go to https://code.visualstudio.com/docs/setup/linux
    - VS Code Extensions:
        - C/C++ https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
        - Dev Containers https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers
            - NB! Make sure Dev containers plugin is pointing to Podman, if you are not using Docker. Check `$HOME/.config/Code/User/settings.json` (for Linux).


## Build & Run

The commands below run inside the dev container.

Build:
```sh
make -C src
```

Run (start webfsd):
```sh
make -C src serve
```

Then open a web browser and check `http//127.0.0.1:8080/`


## Stop & Clean

Stop webfsd:
```sh
make -C src stop
```

Clean build artifacts:
```sh
make -C src clean
```

## License

MIT License, see `LICENSE` for details.