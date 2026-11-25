FROM debian:13.7@sha256:9c6e381421d0f48d1b8133e573f33c185ee16cbf505eda9c4a8a4791b25ea8a6

# TO DO: Adjust the following as needed for your development environment
# - Get host OS user UID/GID and set them as build args to avoid permission issues with mounted volumes

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Refresh the package index and install tools in the same layer.
# webfs and media-types provide the static server and .wasm MIME type for `make serve`.
RUN apt-get update \
    && apt-get install -y \
        bear \
        ca-certificates \
        clang \
        clang-format-19 \
        clang-tidy \
        clangd-19 \
        cppcheck \
        curl \
        editorconfig \
        gdb \
        git \
        lcov \
        libclang-rt-dev \
        libgmock-dev \
        libgtest-dev \
        lld \
        llvm \
        locales \
        make \
        media-types \
        nodejs \
        npm \
        pkg-config \
        sudo \
        util-linux \
        valgrind \
        wabt \
        webfs \
    && rm -rf /var/lib/apt/lists/*

# Generate UTF-8 locale so host LANG/LC_* env vars (forwarded by VS Code) resolve
RUN sed -i 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && locale-gen
ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

# If you have permission issues due to running a root in container, you can run the
# following command inside the container: sudo chown -R $(whoami):$(whoami) /workspace

# Prepare User
ARG USERNAME=engemil

# Create the group first to avoid "group already exists" error
RUN groupadd $USERNAME
# Create user 'engemil' with home directory and bash shell
RUN useradd -m -s /bin/bash -g $USERNAME $USERNAME
# Add user to sudo group
RUN usermod -aG sudo $USERNAME
# Allow passwordless sudo (typical for dev containers)
RUN echo "engemil ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Switch to the new user
USER $USERNAME
WORKDIR /home/$USERNAME

## Here you can add any additional tools or dependencies that you want to install as the non-root user with sudo privileges.



# Make workspace directory
RUN sudo mkdir /workspace && sudo chown $USERNAME:$USERNAME /workspace
WORKDIR /workspace
