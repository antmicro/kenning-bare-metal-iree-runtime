#!/usr/bin/env bash

# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

ROOT_DIR=$(dirname $(dirname $(realpath $0)))

if [[ -z "${IREE_HOST_BIN_DIR=}" ]]; then
    IREE_HOST_BIN_DIR="${ROOT_DIR?}/build/iree_compiler/bin" 
fi

if [[ -z "${RISCV_TOOLCHAIN_ROOT=}" ]]; then
    RISCV_TOOLCHAIN_ROOT="${ROOT_DIR?}/build/toolchain_iree_rv32imf"
fi

if [[ -z "${RENODE_PATH=}" ]]; then
    RENODE_PATH="${ROOT_DIR?}/build/renode"
fi

if [[ $@ == *"-DMINIMAL_BUILD=True"* ]]; then
    CMAKE_TOOLCHAIN_FILE=""
else
    CMAKE_TOOLCHAIN_FILE="${ROOT_DIR?}/third-party/iree-rv32-springbok/cmake/riscv_iree.cmake"
fi

cmake \
    -B build/build-riscv \
    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DIREE_HOST_BIN_DIR="${IREE_HOST_BIN_DIR}" \
    -DRISCV_TOOLCHAIN_ROOT="${RISCV_TOOLCHAIN_ROOT}" \
    -DRENODE_PATH="${RENODE_PATH}" \
    $@
