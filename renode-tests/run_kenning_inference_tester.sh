#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_DIR=$(dirname $SCRIPT_DIR)

cd $PROJECT_DIR/third-party/kenning
python3 -m kenning.scenarios.json_inference_tester \
    ./scripts/jsonconfigs/iree-bare-metal-inference.json \
    $SCRIPT_DIR/results/kenning_output.json
