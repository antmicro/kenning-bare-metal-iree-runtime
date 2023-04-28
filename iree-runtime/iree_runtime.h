/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_IREE_RUNTIME_H_
#define IREE_RUNTIME_IREE_RUNTIME_H_

#if !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))
#include "springbok.h"
#else // !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))
#include "mocks/springbok.h"
#endif // !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))

#include "utils/model.h"
#include "utils/protocol.h"

#define VALIDATE_REQUEST(callback_message_type, request)       \
    if (!IS_VALID_POINTER(request))                            \
    {                                                          \
        return RUNTIME_STATUS_INVALID_POINTER;                 \
    }                                                          \
    if ((callback_message_type) != (*(request))->message_type) \
    {                                                          \
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;            \
    }

#define CHECK_MODEL_STATUS_LOG(status, response, log_format, log_args...)                \
    if (MODEL_STATUS_OK != status)                                                       \
    {                                                                                    \
        LOG_ERROR(log_format, ##log_args);                                               \
        RETURN_ON_ERROR(prepare_failure_response(request), RUNTIME_STATUS_SERVER_ERROR); \
        return RUNTIME_STATUS_MODEL_ERROR;                                               \
    }                                                                                    \
    LOG_DEBUG(log_format, ##log_args);

/**
 * An enum that describes runtime status
 */
#define RUNTIME_STATUSES(STATUS)           \
    STATUS(RUNTIME_STATUS_OK)              \
    STATUS(RUNTIME_STATUS_ERROR)           \
    STATUS(RUNTIME_STATUS_SERVER_ERROR)    \
    STATUS(RUNTIME_STATUS_MODEL_ERROR)     \
    STATUS(RUNTIME_STATUS_INVALID_POINTER) \
    STATUS(RUNTIME_STATUS_INVALID_MESSAGE_TYPE)

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
#define CALLBACKS                                    \
    /*    MessageType           Callback_function */ \
    ENTRY(MESSAGE_TYPE_OK, ok_callback)              \
    ENTRY(MESSAGE_TYPE_ERROR, error_callback)        \
    ENTRY(MESSAGE_TYPE_DATA, data_callback)          \
    ENTRY(MESSAGE_TYPE_MODEL, model_callback)        \
    ENTRY(MESSAGE_TYPE_PROCESS, process_callback)    \
    ENTRY(MESSAGE_TYPE_OUTPUT, output_callback)      \
    ENTRY(MESSAGE_TYPE_STATS, stats_callback)        \
    ENTRY(MESSAGE_TYPE_IOSPEC, iospec_callback)

#define ENTRY(msg_type, callback_func) RUNTIME_STATUS callback_func(message **);
CALLBACKS
#undef ENTRY

#endif // IREE_RUNTIME_IREE_RUNTIME_H_
