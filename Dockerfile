FROM ubuntu:22.04 AS builder

ENV INST 'apt-get install -y --no-install-recommends'
ENV PIPINST 'python3 -m pip install --no-cache-dir --upgrade'

RUN apt-get update && $INST \
    build-essential \
    git \
    libpython3-dev \
    python3 \
    python3-dev \
    python3-pip \
    wget \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY ./third-party/iree-rv32-springbok/build_tools /build_tools
COPY ./requirements.txt /build_tools
COPY ./third-party/kenning /build_tools/kenning

WORKDIR /build_tools

RUN $PIPINST pip setuptools wheel
RUN $PIPINST -r requirements.txt

RUN ./install_toolchain.sh toolchain_backups/toolchain_iree_rv32_20220307.tar.gz
RUN ./download_iree_compiler.py
RUN ./download_renode.py

RUN pip3 install /build_tools/kenning[tensorflow,reports,uart,renode,object_detection]

RUN $PIPINST -r build/renode/tests/requirements.txt

FROM ubuntu:22.04

ENV INST 'apt-get install -y --no-install-recommends'
ENV PIPINST 'python3 -m pip install --no-cache-dir --upgrade'

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && $INST \
    build-essential \
    cmake \
    fonts-lato \
    git \
    git-lfs \
    libglfw3 \
    libglfw3-dev \
    libglib2.0-0 \
    libpython3-dev \
    libtinfo5 \
    mono-complete=6.8.0.105+dfsg-3.2 \
    ninja-build \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    ruby \
    wget \
    xxd \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build_tools/build/renode/renode /usr/bin/renode
COPY --from=builder /build_tools/build/iree_compiler/ /usr/local/iree_compiler
COPY --from=builder /build_tools/build/toolchain_iree_rv32imf /usr/local/toolchain_iree_rv32imf
COPY --from=builder /usr/local /usr/local

RUN $PIPINST \
    iree-compiler~=20230209.425 \
    iree-runtime~=20230209.425 \
    iree-tools-tf~=20230209.425 \
    iree-tools-tflite~=20230209.425 \
    jupyter-book

RUN gem install ceedling
