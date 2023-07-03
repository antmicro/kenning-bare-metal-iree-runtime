# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

iree_add_all_subdirs()

set(RUNTIME_NAME "iree_runtime")
set(RUNTIME_DEPS)

list(APPEND RUNTIME_DEPS iree::modules::hal)
list(APPEND RUNTIME_DEPS ::utils::model)
list(APPEND RUNTIME_DEPS ::utils::protocol)
list(APPEND RUNTIME_DEPS ::utils::uart)
list(APPEND RUNTIME_DEPS ::utils::i2c)
if (DEFINED I2C_ACCELEROMETER)
  set(RUNTIME_NAME "${RUNTIME_NAME}_i2c_accelerometer")
  list(APPEND RUNTIME_DEPS ::utils::sensor_input_reader)
else (DEFINED I2C_ACCELEROMETER)
  list(APPEND RUNTIME_DEPS ::utils::base_input_reader)
endif (DEFINED I2C_ACCELEROMETER)

iree_cc_binary(
  NAME
    "${RUNTIME_NAME}"
  HDRS
    "iree_runtime.h"
  SRCS
    "iree_runtime.c"
  DEPS
    "${RUNTIME_DEPS}"
  LINKOPTS
    "LINKER:--defsym=__itcm_length__=1M"
    "LINKER:--defsym=__stack_size__=200k"
)
