cd third-party/kenning
source ./venv/bin/activate
python3 -m kenning.scenarios.json_inference_tester ./scripts/jsonconfigs/iree-bare-metal-inference.json ./output.json
