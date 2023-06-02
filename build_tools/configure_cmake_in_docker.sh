#!/usr/bin/env bash

# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

ROOT_DIR=$(dirname $(dirname $(realpath $0)))

cmake \
    -B build/build-riscv \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR?}/third-party/iree-rv32-springbok/cmake/riscv_iree.cmake" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DIREE_HOST_BIN_DIR="/usr/local/iree_compiler/bin" \
    -DRISCV_TOOLCHAIN_ROOT="/usr/local/toolchain_iree_rv32imf" \
    $@
