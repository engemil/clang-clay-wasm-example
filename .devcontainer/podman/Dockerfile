# Fully qualified reference avoids Podman's short-name registry resolution.
FROM docker.io/library/debian:13.7@sha256:9c6e381421d0f48d1b8133e573f33c185ee16cbf505eda9c4a8a4791b25ea8a6

# TO DO: Adjust the following as needed for your development environment
# - Get host OS user UID/GID and set them as build args to avoid permission issues with mounted volumes

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
# Update and basic dependencies
RUN apt-get update
RUN apt-get install locales -y
RUN apt-get install sudo -y
RUN apt-get install git -y

# Install C/C++ build tools and development tools
#RUN apt-get install build-essential -y
RUN apt-get install make -y
RUN apt-get install pkg-config -y
RUN apt-get install clang lld llvm -y
RUN apt-get install wabt -y
# Editor intelligence and code navigation tools
RUN apt-get install clangd-19 -y
RUN apt-get install bear -y
# Formatting tools
RUN apt-get install clang-format-19 -y
RUN apt-get install editorconfig -y
# Debugging tools
RUN apt-get install gdb -y
# static analysis tools
RUN apt-get install cppcheck -y
RUN apt-get install clang-tidy -y
# Dynamic analysis tools
RUN apt-get install valgrind -y
RUN apt-get install libclang-rt-dev -y
# Testing and coverage tools
RUN apt-get install libgtest-dev -y
RUN apt-get install libgmock-dev -y
RUN apt-get install lcov -y
# Additional tools for code coverage
RUN apt-get install -y nodejs npm

# Application related dependencies
# lightweight C static server for `make serve` (.wasm MIME via /etc/mime.types)
RUN apt-get install webfs media-types -y

# Extra tools for development
RUN apt-get install curl util-linux -y
RUN apt-get install ca-certificates -y

# Clean up
RUN rm -rf /var/lib/apt/lists/*

# Generate UTF-8 locale so host LANG/LC_* env vars (forwarded by VS Code) resolve
RUN sed -i 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && locale-gen
ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# If you have permission issues due to running a root in container, you can run the
# following command inside the container: sudo chown -R $(whoami):$(whoami) /workspace

ARG USERNAME=engemil

# Keep these IDs aligned with user and userns_mode in docker-compose.yml.
# Podman maps the invoking host user's UID/GID to these container IDs.
RUN groupadd --gid 1000 "${USERNAME}" \
    && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash "${USERNAME}" \
    && printf '%s ALL=(ALL) NOPASSWD:ALL\n' "${USERNAME}" > /etc/sudoers.d/devcontainer \
    && chmod 0440 /etc/sudoers.d/devcontainer

# Switch to the new user
USER $USERNAME
WORKDIR /home/$USERNAME

## Here you can add any additional tools or dependencies that you want to install as the non-root user with sudo privileges.



# Clean up
RUN rm -rf /var/lib/apt-get/lists/*

# Make workspace directory
RUN sudo mkdir /workspace && sudo chown $USERNAME:$USERNAME /workspace
WORKDIR /workspace
