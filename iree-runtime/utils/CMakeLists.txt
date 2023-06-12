# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

iree_cc_library(
  NAME
    utils
  HDRS
    "utils.h"
  SRCS
    "utils.c"
)

iree_cc_library(
  NAME
    model
  HDRS
    "model.h"
  SRCS
    "model.c"
  DEPS
    ::utils
    ::iree_wrapper
    springbok
)

iree_cc_library(
  NAME
    iree_wrapper
  HDRS
    "iree_wrapper.h"
  SRCS
    "iree_wrapper.c"
  DEPS
    ::utils
    iree::hal::drivers::local_sync::sync_driver
    iree::hal::local::loaders::embedded_elf_loader
    iree::modules::hal
    iree::vm::bytecode_module
    springbok
)

iree_cc_library(
  NAME
    protocol
  HDRS
    "protocol.h"
  SRCS
    "protocol.c"
  DEPS
    ::utils
    ::uart
)

iree_cc_library(
  NAME
    uart
  HDRS
    "uart.h"
  SRCS
    "uart.c"
  DEPS
    ::utils
)

if (DEFINED I2C_ACCELEROMETER)

iree_cc_library(
  NAME
    i2c
  HDRS
    "i2c.h"
  SRCS
    "i2c.c"
  DEPS
    ::utils
)

iree_cc_library(
  NAME
    adxl345
  HDRS
    "adxl345.h"
  SRCS
    "adxl345.c"
  DEPS
    ::utils
    ::i2c
)

iree_cc_library(
  NAME
    sensor
  HDRS
    "sensor.h"
  SRCS
    "sensor.c"
  DEPS
    ::utils
    ::i2c
    ::adxl345
    springbok
)

endif (DEFINED I2C_ACCELEROMETER)
