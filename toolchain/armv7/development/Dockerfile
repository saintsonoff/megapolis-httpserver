FROM --platform=linux/arm/v7 arm32v7/debian:trixie

RUN apt-get update -y && apt-get upgrade -y && \
apt-get \
    install \
    libssl-dev \
    g++ \
    cmake \
    libssl-dev \
    libboost-all-dev \
    ninja-build \
    gdb \
    libatomic1 \
    curl \
    sudo \
    build-essential \
    clangd \
    -y

RUN echo "deb http://deb.debian.org/debian/ sid main" | sudo tee /etc/apt/sources.list.d/sid.list && \
    apt-get update -y && apt-get upgrade -y

CMD ["/bin/bash"]
