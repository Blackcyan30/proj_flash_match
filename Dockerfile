FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Target architecture for Miniconda installer (arm64 or x86_64)
ARG ARCH=arm64

RUN apt-get update && \
    apt-get install -y build-essential cmake git curl wget ca-certificates vim && \
    rm -rf /var/lib/apt/lists/*

# Map arm64 to aarch64 for the Linux installer
RUN if [ "$ARCH" = "arm64" ]; then \
        ARCH_URL="aarch64"; \
    else \
        ARCH_URL="$ARCH"; \
    fi && \
    wget -O /tmp/miniconda.sh https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-${ARCH_URL}.sh && \
    bash /tmp/miniconda.sh -b -p /opt/conda && \
    rm /tmp/miniconda.sh

ENV PATH=/opt/conda/bin:$PATH

RUN conda install -y python=3.11.5 matplotlib numpy && \
    conda clean -afy

WORKDIR /workspace
