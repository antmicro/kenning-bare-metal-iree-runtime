#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_DIR=$(dirname $SCRIPT_DIR)

cd $PROJECT_DIR/third-party/kenning
python3 -m kenning.scenarios.render_report \
    'iree-runtime' \
    $SCRIPT_DIR/results/benchmarks/iree-runtime.md \
    --measurements $SCRIPT_DIR/results/kenning_output.json \
    --root-dir $SCRIPT_DIR/results/benchmarks \
    --report-types performance classification
