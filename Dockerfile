# FROM ubuntu:24.04
#
# ENV DEBIAN_FRONTEND=noninteractive
#
# # Installing C++ compile, cmake, Git and download tools
# RUN apt-get update && apt-get install -y \
#     build-essential \
#     cmake \
#     git \
#     curl \
#     wget && \
#     rm -rf /var/lib/apt/lists/*
#
# RUN wget -O /tmp/miniconda.sh \
#     https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
#     bash /tmp/miniconda.sh -b -p /opt/conda && \
#     rm /tmp/miniconda.sh
#
# ENV PATH=/opt/conda/bin:$PATH
#
# # Create a conda environment with python 3.11.5 and common libs
# RUN conda install python=3.11.5 matplotlib numpy && \
#     conda clean -afy
#
# WORKDIR /workspace

# FROM ubuntu:24.04
#
# ENV DEBIAN_FRONTEND=noninteractive
#
# # 1. Install essential build tools
# RUN apt-get update && \
#     apt-get install -y \
#     build-essential cmake git curl wget ca-certificates && \
#     rm -rf /var/lib/apt/lists/*
#
# # 2. Download Miniconda installer
# RUN wget -O /tmp/miniconda.sh \
#       https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
#
# # 3. Verify installer (optional; for debugging)
# RUN head -n 5 /tmp/miniconda.sh
#
# # 4. Install Miniconda silently
# RUN bash /tmp/miniconda.sh -b -p /opt/conda
#
# # 5. Cleanup installer
# RUN rm /tmp/miniconda.sh
#
# ENV PATH=/opt/conda/bin:$PATH
#
# # 6. Install Python and libraries
# RUN conda install -y python=3.11.5 matplotlib numpy && \
#     conda clean -afy
#
# WORKDIR /workspace


FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential cmake git curl wget ca-certificates && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/miniconda.sh \
    https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-aarch64.sh

RUN head -n 5 /tmp/miniconda.sh

RUN bash /tmp/miniconda.sh -b -p /opt/conda

RUN rm /tmp/miniconda.sh

ENV PATH=/opt/conda/bin:$PATH

RUN conda install -y python=3.11.5 matplotlib numpy && \
    conda clean -afy

WORKDIR /workspace
