FROM debian:bullseye

ENV INST 'apt-get install -y --no-install-recommends'
ENV PIPINST 'python3 -m pip install --no-cache-dir --upgrade'
ENV REPO_PATH '/kenning-bare-metal-iree-runtime'

RUN apt-get update && $INST \
    build-essential \
    clang-format \
    clang-tidy \
    fonts-lato \
    git \
    git-lfs \
    libncurses5 \
    ninja-build \
    python3 \
    python3-pip \
    ruby \
    wget \
    xxd \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/antmicro/kenning-bare-metal-iree-runtime $REPO_PATH && \
    cd $REPO_PATH && \
    git submodule update --init --recursive && \
    mkdir -p build

ADD docker-image/third-party/iree-compiler.tar.gz $REPO_PATH/build/
ADD docker-image/third-party/iree-toolchain.tar.gz $REPO_PATH/build/

RUN wget -P /tmp/ https://dl.antmicro.com/projects/renode/builds/renode-latest.linux-portable.tar.gz && \
    tar -C /tmp/ -xvzf /tmp/renode-latest.linux-portable.tar.gz && \
    mv /tmp/renode_*/ $REPO_PATH/build/renode/ && \
    rm /tmp/renode*.tar.gz

RUN $PIPINST pip setuptools PyYAML

RUN $PIPINST \
    -r $REPO_PATH/build/renode/tests/requirements.txt \
    iree-compiler~=20230209.425 \
    iree-runtime~=20230209.425 \
    iree-tools-tf~=20230209.425 \
    iree-tools-tflite~=20230209.425

RUN $PIPINST $(find $REPO_PATH/build/iree_compiler/tmp -name "iree_tools_tflite*.whl")

RUN $PIPINST $REPO_PATH/third-party/kenning[tensorflow,reports,uart,renode]

RUN $PIPINST \
    charset-normalizer==3.1.0 \
    cmake \
    git+https://github.com/antmicro/pyrenode \
    git+https://github.com/antmicro/renode-run \
    git+https://github.com/antmicro/servis#egg=servis[bokeh,matplotlib] \
    google \
    iree_compiler~=20230209.425 \
    iree_runtime~=20230209.425 \
    iree-tools-tf~=20230209.425 \
    jsonschema \
    numpy \
    pillow \
    protobuf \
    psutil \
    pyserial \
    requests \
    tensorflow \
    tqdm \
    wget

RUN gem install ceedling
