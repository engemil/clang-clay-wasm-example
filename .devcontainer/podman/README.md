# Podman dev container

Note that the Compose filename and Dockerfile name are retained for Dev Containers compatibility; neither requires the Docker engine.

References:
- [Podman user namespaces and volume labels](https://docs.podman.io/en/latest/markdown/podman-run.1.html)
- [VS Code Podman support and Compose providers](https://code.visualstudio.com/remote/advancedcontainers/docker-options#_podman)


## Host setup

Install Podman and a recent `podman-compose` provider supporting `userns_mode: "keep-id:uid=1000,gid=1000"`.

The rootless host account needs valid subordinate UID/GID ranges in `/etc/subuid` and `/etc/subgid`, and the host's rootless Podman prerequisites (including `newuidmap` and `newgidmap`).

Run as your normal user, not through `sudo`.

In **VS Code User settings**, select the host-side executables:

```json
{
    "dev.containers.dockerPath": "podman",
    "dev.containers.dockerComposePath": "podman-compose"
}
```

These are host settings, not settings inside the container. Using `podman-compose` directly avoids needing a Docker-compatible API socket or an ambiguous `podman compose` provider selection.

Do not put `podman compose` (a command plus subcommand) in an executable-path setting.

Open the repository in VS Code, use **Dev Containers: Reopen in Container**, and select `.devcontainer/podman/devcontainer.json` when prompted for a configuration. The existing Docker configuration remains separate.


