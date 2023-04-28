#!/usr/bin/env bash

# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

ROOTDIR=$(dirname $(dirname $(realpath $0)))

command="start;"
if [[ "$1" == "debug" ]]; then
    command="machine StartGdbServer 3333;"
fi

bin_file=$(realpath build/build-riscv/iree-runtime/iree_runtime)
(cd "${ROOTDIR}" && ./build/renode/renode -e "\$bin=@${bin_file}; i @sim/config/springbok.resc; \
${command} sysbus.vec_controlblock WriteDoubleWord 0xc 0" \
    --disable-xwt --console)
