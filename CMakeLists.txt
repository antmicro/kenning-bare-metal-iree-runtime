# Copyright 2023 Antmicro <www.antmicro.com
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.4)

# Project configuration

project(iree-runtime)

include_guard(GLOBAL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SPRINGBOK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party/iree-rv32-springbok" CACHE PATH
    "Springbok source code path." FORCE)
set(IREE_SOURCE_DIR "${SPRINGBOK_SOURCE_DIR}/third_party/iree" CACHE PATH
    "IREE source code path." FORCE)
set(UNIT_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/unit-tests" CACHE PATH
    "Unit tests directory." FORCE)
set(RENODE_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/renode-tests" CACHE PATH
    "Renode tests directory." FORCE)
set(RENODE_TEST_BIN_PATH "${RENODE_PATH}/renode-test" CACHE PATH
    "Path to renode-test")

if (I2C_ACCELEROMETER)
    add_compile_definitions(I2C_SENSOR)
    add_compile_definitions(I2C_ADXL345)
endif (I2C_ACCELEROMETER)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_LIST_DIR})
include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

if (NOT MINIMAL_BUILD)
    # IREE-specific settings
    set(IREE_BUILD_COMPILER OFF)
    set(IREE_ENABLE_MLIR OFF)
    set(IREE_BUILD_SAMPLES OFF)
    set(IREE_ENABLE_THREADING OFF)
    set(IREE_BUILD_TESTS OFF)
    set(IREE_BUILD_BINDINGS_TFLITE OFF)
    set(IREE_HAL_DRIVER_LOCAL_SYNC ON CACHE BOOL "" FORCE)
    set(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE ON CACHE BOOL "" FORCE)
    set(IREE_HAL_EXECUTABLE_LOADER_EMBEDDED_ELF ON CACHE BOOL "" FORCE)
    set(IREE_HAL_EXECUTABLE_LOADER_DEFAULTS OFF CACHE BOOL "" FORCE)
    set(IREE_HAL_DRIVER_DEFAULTS OFF CACHE BOOL "" FORCE)

    # IREE-specific definitions
    set(SPRINGBOK_CONFIG_HEADER "${SPRINGBOK_SOURCE_DIR}/springbok_config.h")
    add_definitions(-DFLATCC_USE_GENERIC_ALIGNED_ALLOC)
    add_definitions(-DIREE_PLATFORM_GENERIC)
    add_definitions(-DIREE_SYNCHRONIZATION_DISABLE_UNSAFE=1)
    add_definitions(-DIREE_FILE_IO_ENABLE=0)
    add_definitions(-DIREE_USER_CONFIG_H="${SPRINGBOK_CONFIG_HEADER}")
    add_definitions(-DIREE_VM_EXECUTION_TRACING_ENABLE=1)

    message("Include IREE source at ${IREE_SOURCE_DIR}")
    add_subdirectory(${IREE_SOURCE_DIR} iree)

    message("Include Springbok source at ${SPRINGBOK_SOURCE_DIR}")
    add_subdirectory(${SPRINGBOK_SOURCE_DIR} iree-riscv32)

    set(SPRINGBOK_ROOT_DIR "${SPRINGBOK_SOURCE_DIR}" CACHE PATH "Springbok Root directory")

    add_subdirectory(iree-runtime iree-runtime)
endif (NOT MINIMAL_BUILD)

set(FIND_SRC_FILES "find \
    ${CMAKE_CURRENT_SOURCE_DIR}/iree-runtime \
    -iname \"*.c\" -o -iname \"*.h\"")
set(FIND_SRC_AND_TEST_FILES "find \
    ${CMAKE_CURRENT_SOURCE_DIR}/iree-runtime \
    ${UNIT_TESTS_DIR}/tests \
    -iname \"*.c\" -o -iname \"*.h\"")

add_custom_target(clang-tidy-fix
    COMMAND bash -c "rm -f ${CMAKE_BINARY_DIR}/compile_commands.json && \
        clang-tidy -format-style=file --use-color -fix \$(${FIND_SRC_FILES}) -- \
        -isystem ${SPRINGBOK_SOURCE_DIR}/springbok/include \
        -isystem ${IREE_SOURCE_DIR}/runtime/src \
        -isystem ${UNIT_TESTS_DIR}/tests \
        -D__CLANG_TIDY__"
    USES_TERMINAL
    VERBATIM
)
add_custom_target(clang-tidy
    COMMAND bash -c "rm -f ${CMAKE_BINARY_DIR}/compile_commands.json && \
        clang-tidy -format-style=file --use-color \$(${FIND_SRC_FILES}) -- \
        -isystem ${SPRINGBOK_SOURCE_DIR}/springbok/include \
        -isystem ${IREE_SOURCE_DIR}/runtime/src \
        -isystem ${UNIT_TESTS_DIR}/tests \
        -D__CLANG_TIDY__ \
        -DI2C_SENSOR \
        -DI2C_ADXL345"
    USES_TERMINAL
    VERBATIM
)
add_custom_target(clang-format-fix
    COMMAND bash -c "${FIND_SRC_AND_TEST_FILES} | xargs \
        clang-format --style=file -i"
    USES_TERMINAL
    VERBATIM
)
add_custom_target(clang-format
    COMMAND bash -c "${FIND_SRC_AND_TEST_FILES} | xargs \
        clang-format --dry-run --Werror --style=file -i"
    USES_TERMINAL
    VERBATIM
)
add_custom_target(unit-tests
    COMMAND bash -c "ceedling test:all"
    USES_TERMINAL
    VERBATIM
    USES_TERMINAL
    WORKING_DIRECTORY ${UNIT_TESTS_DIR}
)
add_custom_target(unit-tests-coverage
    COMMAND bash -c "ceedling gcov:all || exit 0"
    COMMAND bash -c "ceedling utils:gcov"
    USES_TERMINAL
    VERBATIM
    USES_TERMINAL
    WORKING_DIRECTORY ${UNIT_TESTS_DIR}
)
add_custom_target(renode-tests
    COMMAND bash -c "find . -iname test_*.robot | xargs ${RENODE_TEST_BIN_PATH} --show-log -r results"
    VERBATIM
    USES_TERMINAL
    WORKING_DIRECTORY ${RENODE_TESTS_DIR}
)

