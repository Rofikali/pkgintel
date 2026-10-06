# FROM ubuntu:24.04

# ENV DEBIAN_FRONTEND=noninteractive

# RUN apt-get update && \
#     apt-get install -y --no-install-recommends \
#         build-essential \
#         clang \
#         clang-format \
#         clang-tidy \
#         cmake \
#         ninja-build \
#         gdb \
#         valgrind \
#         git \
#         file \
#         pkg-config \
#         python3 \
#         python3-pip \
#         ca-certificates \
#         sudo \
#         less \
#         vim \
#         nano \
#         strace \
#         ltrace \
#         linux-libc-dev \
#         libc6-dev \
#     && rm -rf /var/lib/apt/lists/*

# # Non-root development user.
# ARG USERNAME=developer
# ARG USER_UID=1001
# ARG USER_GID=1001

# RUN groupadd --gid ${USER_GID} ${USERNAME} && \
#     useradd \
#         --uid ${USER_UID} \
#         --gid ${USER_GID} \
#         --create-home \
#         --shell /bin/bash \
#         ${USERNAME} && \
#     echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} && \
#     chmod 0440 /etc/sudoers.d/${USERNAME}

# WORKDIR /workspace/pkgintel

# USER ${USERNAME}

# CMD ["bash"]


FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    ninja-build \
    gdb \
    valgrind \
    git \
    file \
    pkg-config \
    python3 \
    python3-pip \
    ca-certificates \
    sudo \
    less \
    vim \
    nano \
    strace \
    ltrace \
    linux-libc-dev \
    libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Non-root development user configuration.
ARG USERNAME=developer
ARG USER_UID=1001
ARG USER_GID=1001

RUN groupadd --gid ${USER_GID} ${USERNAME} && \
    useradd \
    --uid ${USER_UID} \
    --gid ${USER_GID} \
    --create-home \
    --shell /bin/bash \
    ${USERNAME} && \
    echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME}

WORKDIR /workspace/pkgintel

USER ${USERNAME}

# Configures Git inside the container to safely trust your mounted Windows directories
RUN git config --global --add safe.directory '*'

CMD ["bash"]
