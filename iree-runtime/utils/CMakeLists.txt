
iree_cc_library(
  NAME
    model
  HDRS
    "model.h"
  SRCS
    "model.c"
  DEPS
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
    ::uart
)

iree_cc_library(
  NAME
    uart
  HDRS
    "uart.h"
  SRCS
    "uart.c"
)