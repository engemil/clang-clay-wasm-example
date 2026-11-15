# Fully qualified reference avoids Podman's short-name registry resolution.
FROM docker.io/library/debian:13.7@sha256:9c6e381421d0f48d1b8133e573f33c185ee16cbf505eda9c4a8a4791b25ea8a6

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        locales \
        sudo \
        build-essential \
        pkg-config \
        clang \
        lld \
        webfs \
        media-types \
        git \
        curl \
        ca-certificates \
    && sed -i 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && locale-gen \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

ARG USERNAME=engemil

# Keep these IDs aligned with user and userns_mode in docker-compose.yml.
# Podman maps the invoking host user's UID/GID to these container IDs.
RUN groupadd --gid 1000 "${USERNAME}" \
    && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash "${USERNAME}" \
    && printf '%s ALL=(ALL) NOPASSWD:ALL\n' "${USERNAME}" > /etc/sudoers.d/devcontainer \
    && chmod 0440 /etc/sudoers.d/devcontainer \
    && install -d -o "${USERNAME}" -g "${USERNAME}" /workspace

USER ${USERNAME}
WORKDIR /workspace
