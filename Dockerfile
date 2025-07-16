FROM ubuntu:24.04

ARG ARCH=x86_64
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y build-essential cmake git curl wget ca-certificates vim && \
    rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/miniconda.sh \
    https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-${ARCH}.sh

RUN bash /tmp/miniconda.sh -b -p /opt/conda && rm /tmp/miniconda.sh

ENV PATH=/opt/conda/bin:$PATH

RUN conda install -y python=3.11.5 matplotlib numpy && \
    conda clean -afy

WORKDIR /workspace
