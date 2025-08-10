FROM ubuntu:24.04

ARG ARCH=x86_64
ARG GRPC_VERSION=v1.74.0
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install system dependencies
RUN apt-get update && \
    apt-get install -y \
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
    zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

# Build gRPC and Protobuf from source to get specific versions
WORKDIR /tmp

# Clone the gRPC repository with a specific version tag
# IMPORTANT: We use this specific version (v1.74.0 by default) because:
# 1. It's compatible with C++20 when properly configured
# 2. C++20 deprecates certain string literal to const char* conversions used in older Protobuf code
# 3. We need to maintain version consistency to ensure reproducible builds
RUN git clone --depth 1 --branch ${GRPC_VERSION} https://github.com/grpc/grpc.git && \
    cd grpc && \
    git submodule update --init && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    # Configure with C++20 support and handle deprecation warnings
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=20 \
          -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          -DCMAKE_INSTALL_PREFIX=/usr \
          -DBUILD_SHARED_LIBS=ON \
          -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DgRPC_SSL_PROVIDER=package \
          -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" \
          ../.. && \
    # Build and install
    make -j$(nproc) && \
    make install && \
    # Clean up
    cd /tmp && rm -rf grpc

# Install Python directly from Ubuntu packages instead of Miniconda
# This avoids architecture compatibility issues
RUN apt-get update && \
    apt-get install -y \
    python3 \
    python3-pip \
    python3-numpy \
    python3-matplotlib && \
    rm -rf /var/lib/apt/lists/*

# Create symlinks for more natural commands
RUN ln -sf /usr/bin/python3 /usr/bin/python && \
    ln -sf /usr/bin/pip3 /usr/bin/pip

# Python is already installed and on PATH, no need for extra ENV settings

# Use apt to install pandas (Ubuntu 24.04 has a system package constraint)
RUN apt-get update && \
    apt-get install -y python3-pandas && \
    rm -rf /var/lib/apt/lists/*

# Verify gRPC and Protobuf installation
RUN echo "---- Build Environment Details ----" && \
    echo "Architecture: $(uname -m)" && \
    echo "Protobuf: $(protoc --version)" && \
    echo "gRPC: $(grep -oP '(?<=GRPC_CPP_VERSION )[0-9]+\.[0-9]+\.[0-9]+' /usr/include/grpc/grpc.h)" && \
    echo "C++ Standard: C++20" && \
    echo "Compiler: $(c++ --version | head -n 1)" && \
    echo "--------------------------------"

# Set the working directory for the project
WORKDIR /flashmatch

# Copy the project files into the container
# COPY . .

# Note: We don't copy files by default - we'll use volume mounting
# This allows for live development in the container

# Just provide a bash shell without automatic building
CMD ["bash"]
