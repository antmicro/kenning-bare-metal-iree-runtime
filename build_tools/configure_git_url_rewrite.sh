#!/usr/bin/env bash

# Those rewrites are required as some repos were moved

git config --global url."https://github.com/tensorflow/mlir-hlo".insteadOf "https://github.com/iree-org/iree-mhlo-fork.git"
git config --global url."https://github.com/iree-org/llvm-project.git".insteadOf "https://github.com/iree-org/iree-llvm-fork.git"
