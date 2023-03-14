
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
  LINKOPTS
    "LINKER:--defsym=__itcm_length__=1M"
    "LINKER:--defsym=__stack_size__=200k"
)