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

#include "utils/i2c.h"
#include "utils/input_reader.h"
#include "utils/model.h"
#include "utils/protocol.h"
#include "utils/utils.h"

#define VALIDATE_REQUEST(callback_message_type, request)       \
    if (!IS_VALID_POINTER(request))                            \
    {                                                          \
        return RUNTIME_STATUS_INV_PTR;                         \
    }                                                          \
    if ((callback_message_type) != (*(request))->message_type) \
    {                                                          \
        return RUNTIME_STATUS_INV_MSG_TYPE;                    \
    }

#define CHECK_STATUS_LOG(status, response, log_format, log_args...) \
    if (STATUS_OK != status)                                        \
    {                                                               \
        LOG_ERROR(log_format, ##log_args);                          \
        status_t resp_status = prepare_failure_response(request);   \
        RETURN_ON_ERROR(resp_status, resp_status);                  \
        return status;                                              \
    }                                                               \
    LOG_DEBUG(log_format, ##log_args);

/**
 * Runtime custom error codes
 */
#define RUNTIME_STATUSES(STATUS) STATUS(RUNTIME_STATUS_INV_MSG_TYPE)

GENERATE_MODULE_STATUSES(RUNTIME);

#define CHECK_INIT_STATUS_RET(status, log_format, log_args...) \
    if (STATUS_OK != status)                                   \
    {                                                          \
        LOG_ERROR(log_format, ##log_args);                     \
        return false;                                          \
    }                                                          \
    LOG_DEBUG(log_format, ##log_args);

/**
 * Initializes UART
 *
 * @param config UART configuration
 *
 * @returns status of initialization
 */
status_t __attribute__((weak)) uart_init(void *config);

/**
 * Initializes I2C
 *
 * @param config I2C configuration
 *
 * @returns status of the I2C
 */
status_t __attribute__((weak)) i2c_init(void *config);

/**
 * Initializes sensor
 *
 * @returns status of the sensor
 */
status_t __attribute__((weak)) sensor_init(void);

/**
 * Type of callback function
 */
typedef status_t (*callback_ptr)(message_t **);

/**
 * List of callbacks for each message type
 */
#define CALLBACKS(ENTRY)                             \
    /*    MessageType           Callback_function */ \
    ENTRY(MESSAGE_TYPE_OK, ok_callback)              \
    ENTRY(MESSAGE_TYPE_ERROR, error_callback)        \
    ENTRY(MESSAGE_TYPE_DATA, data_callback)          \
    ENTRY(MESSAGE_TYPE_MODEL, model_callback)        \
    ENTRY(MESSAGE_TYPE_PROCESS, process_callback)    \
    ENTRY(MESSAGE_TYPE_OUTPUT, output_callback)      \
    ENTRY(MESSAGE_TYPE_STATS, stats_callback)        \
    ENTRY(MESSAGE_TYPE_IOSPEC, iospec_callback)

#define ENTRY(msg_type, callback_func) status_t callback_func(message_t **);
CALLBACKS(ENTRY)
#undef ENTRY

#endif // IREE_RUNTIME_IREE_RUNTIME_H_
