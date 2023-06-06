# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

iree_add_all_subdirs()

iree_cc_binary(
  NAME
    iree_runtime
  HDRS
    "iree_runtime.h"
  SRCS
    "iree_runtime.c"
  DEPS
    iree::modules::hal
    ::utils::model
    ::utils::uart
    ::utils::protocol
    ::utils::adxl345
    ::utils::i2c
  LINKOPTS
    "LINKER:--defsym=__itcm_length__=1M"
    "LINKER:--defsym=__stack_size__=200k"
)
