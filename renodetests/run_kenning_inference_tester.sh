cd third-party/kenning
git checkout 42337-uart-protocol-for-iree-runtime
python3 -m kenning.scenarios.json_inference_tester ./scripts/jsonconfigs/iree-bare-metal-inference.json ./output.json
