ROOT_DIR=$(realpath $(git rev-parse --show-toplevel))

cmake \
    -B build/build-riscv \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR?}/third-party/iree-rv32-springbok/cmake/riscv_iree.cmake" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DIREE_HOST_BIN_DIR="${ROOT_DIR?}/build/iree_compiler/bin" \
    -DRISCV_TOOLCHAIN_ROOT="${ROOT_DIR?}/build/toolchain_iree_rv32imf" \
    "$@"
