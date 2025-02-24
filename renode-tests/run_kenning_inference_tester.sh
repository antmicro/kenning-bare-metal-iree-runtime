#!/usr/bin/env bash

# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_DIR=$(dirname $SCRIPT_DIR)

export LD_LIBRARY_PATH=${PROJECT_DIR}/build/iree_compiler/lib/:$LD_LIBRARY_PATH
export PATH=${PROJECT_DIR}/build/iree_compiler/bin:$PATH
export IREE_TOOL_PATH=${PROJECT_DIR}/build/iree_compiler/bin

cd $PROJECT_DIR/third-party/kenning
kenning optimize test report \
    --json-cfg $SCRIPT_DIR/../kenning-scenarios/renode-magic-wand-iree-bare-metal-inference.json \
    --measurements $SCRIPT_DIR/results/kenning_output.json \
    --report-name 'iree-runtime' \
    --report-path $SCRIPT_DIR/results/benchmarks/iree-runtime.md \
    --root-dir $SCRIPT_DIR/results/benchmarks \
    --report-types performance classification \
    --verbosity INFO
