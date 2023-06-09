/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_PROTOCOL_H_
#define IREE_RUNTIME_UTILS_PROTOCOL_H_

#include "uart.h"

#define CHECK_UART_STATUS(status)                   \
    if (UART_STATUS_TIMEOUT == (status))            \
    {                                               \
        return PROTOCOL_STATUS_TIMEOUT;             \
    }                                               \
    if (STATUS_OK != (status))                      \
    {                                               \
        return PROTOCOL_STATUS_CLIENT_DISCONNECTED; \
    }

#define MAX_MESSAGE_SIZE_BYTES (5 * 256 * 1024) // 1.25 MB

#define MESSAGE_SIZE_PAYLOAD(msg_size) ((msg_size) - sizeof(message_type_t))
#define MESSAGE_SIZE_FULL(msg_size) (sizeof(message_t) + MESSAGE_SIZE_PAYLOAD(msg_size))

/**
 * An enum that describes message type
 */
#define MESSAGE_TYPES(TYPE)    \
    TYPE(MESSAGE_TYPE_OK)      \
    TYPE(MESSAGE_TYPE_ERROR)   \
    TYPE(MESSAGE_TYPE_DATA)    \
    TYPE(MESSAGE_TYPE_MODEL)   \
    TYPE(MESSAGE_TYPE_PROCESS) \
    TYPE(MESSAGE_TYPE_OUTPUT)  \
    TYPE(MESSAGE_TYPE_STATS)   \
    TYPE(MESSAGE_TYPE_IOSPEC)  \
    TYPE(NUM_MESSAGE_TYPES)

typedef enum
{
    MESSAGE_TYPES(GENERATE_ENUM)
} MESSAGE_TYPE;

/**
 * Protocol custom error codes
 */
#define PROTOCOL_STATUSES(STATUS)               \
    STATUS(PROTOCOL_STATUS_CLIENT_CONNECTED)    \
    STATUS(PROTOCOL_STATUS_CLIENT_DISCONNECTED) \
    STATUS(PROTOCOL_STATUS_CLIENT_IGNORED)      \
    STATUS(PROTOCOL_STATUS_DATA_READY)          \
    STATUS(PROTOCOL_STATUS_DATA_INV)            \
    STATUS(PROTOCOL_STATUS_INTERNAL_ERROR)      \
    STATUS(PROTOCOL_STATUS_MSG_TOO_BIG)

GENERATE_MODULE_STATUSES(PROTOCOL);

typedef uint32_t message_size_t;
typedef uint16_t message_type_t;

/**
 * A struct that contains all parameters describing single message
 */
typedef struct __attribute__((packed))
{
    message_size_t message_size;
    message_type_t message_type;
    uint8_t payload[0];
} message_t;

/**
 * Waits for a message to be received
 *
 * @param msg received message
 *
 * @returns status of the protocol
 */
status_t receive_message(message_t **msg);
/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the protocol
 */
status_t send_message(const message_t *msg);
/**
 * Create a message that indicates an successful action
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t prepare_success_response(message_t **response);
/**
 * Create a message that indicates an error
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t prepare_failure_response(message_t **response);

#endif // IREE_RUNTIME_UTILS_PROTOCOL_H_
