#ifndef IREE_RUNTIME_IREE_RUNTIME_H_
#define IREE_RUNTIME_IREE_RUNTIME_H_

#include "springbok.h"
#include "utils/model.h"
#include "utils/protocol.h"
#include "utils/uart.h"

/**
 * An enum that describes runtime status
 */
#define RUNTIME_STATUSES(STATUS)                                                                                       \
    STATUS(RUNTIME_STATUS_OK)                                                                                          \
    STATUS(RUNTIME_STATUS_ERROR)                                                                                       \
    STATUS(RUNTIME_STATUS_SERVER_ERROR)                                                                                \
    STATUS(RUNTIME_STATUS_MODEL_ERROR)

typedef enum
{
    RUNTIME_STATUSES(GENERATE_ENUM)
} RUNTIME_STATUS;

/**
 * Type of callback function
 */
typedef RUNTIME_STATUS (*callback_ptr)(message **);

/**
 * List of callbacks for each message type
 */
#define CALLBACKS                                                                                                      \
    /*    MessageType           Callback_function */                                                                   \
    ENTRY(MESSAGE_TYPE_OK, ok_callback)                                                                                \
    ENTRY(MESSAGE_TYPE_ERROR, error_callback)                                                                          \
    ENTRY(MESSAGE_TYPE_DATA, data_callback)                                                                            \
    ENTRY(MESSAGE_TYPE_MODEL, model_callback)                                                                          \
    ENTRY(MESSAGE_TYPE_PROCESS, process_callback)                                                                      \
    ENTRY(MESSAGE_TYPE_OUTPUT, output_callback)                                                                        \
    ENTRY(MESSAGE_TYPE_STATS, stats_callback)                                                                          \
    ENTRY(MESSAGE_TYPE_IOSPEC, iospec_callback)

#define ENTRY(msg_type, callback_func) RUNTIME_STATUS callback_func(message **);
CALLBACKS
#undef ENTRY

callback_ptr msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS
#undef ENTRY
};

#endif // IREE_RUNTIME_IREE_RUNTIME_H_
