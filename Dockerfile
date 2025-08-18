# syntax=docker/dockerfile:1.7-labs
# ^ enables BuildKit features like --mount=type=cache (faster builds)
#   https://docs.docker.com/reference/dockerfile/ (Dockerfile reference)
#   https://www.docker.com/blog/new-dockerfile-capabilities-v1-7-0/ (v1.7 features)

FROM ubuntu:24.04

ARG ARCH=x86_64
ARG GRPC_VERSION=v1.74.0

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=UTC

# -------- System deps (+ Python stack) with fast APT + fewer recommends --------
# Cache APT indices across builds; install only what we actually need.
# We combine into one layer to keep images smaller and caching effective.
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      git \
      curl \
      wget \
      ca-certificates \
      vim \
      autoconf \
      libtool \
      pkg-config \
      libssl-dev \
      zlib1g-dev \
      ccache \
      python3 \
      python3-pip \
      python3-numpy \
      python3-matplotlib \
      python3-pandas \
    && rm -rf /var/lib/apt/lists/*

# Natural python/pip names
RUN ln -sf /usr/bin/python3 /usr/bin/python && \
    ln -sf /usr/bin/pip3 /usr/bin/pip

# -------- Build gRPC (C++), from source at the pinned version --------
# Use ccache to speed up repeat builds; keep ccache across builds via BuildKit cache mount.
WORKDIR /tmp
ENV CCACHE_DIR=/root/.cache/ccache
RUN --mount=type=cache,target=/root/.cache/ccache,sharing=locked \
    git clone --depth 1 --branch "${GRPC_VERSION}" https://github.com/grpc/grpc.git && \
    cd grpc && \
    git submodule update --init --depth 1 && \
    mkdir -p cmake/build && cd cmake/build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=20 \
          -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          -DCMAKE_INSTALL_PREFIX=/usr \
          -DBUILD_SHARED_LIBS=ON \
          -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DgRPC_SSL_PROVIDER=package \
          -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
          -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" \
          ../.. && \
    make -j"$(nproc)" && \
    make install && \
    cd /tmp && rm -rf grpc

# -------- (Optional) quick environment report (kept; doesn’t hurt caching) --------
RUN echo "---- Build Environment Details ----" && \
    echo "Architecture: $(uname -m)" && \
    echo "Protobuf: $(protoc --version)" && \
    echo "gRPC (header): $(grep -oP '(?<=GRPC_CPP_VERSION )[0-9]+\\.[0-9]+\\.[0-9]+' /usr/include/grpc/grpc.h || true)" && \
    echo "C++ Standard: C++20" && \
    echo "Compiler: $(c++ --version | head -n 1)" && \
    echo "--------------------------------"

# -------- Project working dir; code is bind-mounted in CI/dev --------
WORKDIR /flashmatch

# We intentionally do NOT COPY the repo here (CI/dev mounts the workspace).
# COPY . .   # (left commented by design)

CMD ["bash"]
