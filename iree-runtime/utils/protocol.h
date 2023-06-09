/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_PROTOCOL_H_
#define IREE_RUNTIME_UTILS_PROTOCOL_H_

#include "uart.h"

#define CHECK_UART_STATUS(status)                 \
    if (UART_STATUS_TIMEOUT == (status))          \
    {                                             \
        return SERVER_STATUS_TIMEOUT;             \
    }                                             \
    if (UART_STATUS_OK != (status))               \
    {                                             \
        return SERVER_STATUS_CLIENT_DISCONNECTED; \
    }

#define MAX_MESSAGE_SIZE_BYTES (5 * 256 * 1024) // 1.25 MB

#define MESSAGE_SIZE_PAYLOAD(msg_size) ((msg_size) - sizeof(message_type_t))
#define MESSAGE_SIZE_FULL(msg_size) (sizeof(message) + MESSAGE_SIZE_PAYLOAD(msg_size))

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
 * An enum that describes server status
 */
#define SERVER_STATUSES(STATUS)               \
    STATUS(SERVER_STATUS_NOTHING)             \
    STATUS(SERVER_STATUS_CLIENT_CONNECTED)    \
    STATUS(SERVER_STATUS_CLIENT_DISCONNECTED) \
    STATUS(SERVER_STATUS_CLIENT_IGNORED)      \
    STATUS(SERVER_STATUS_DATA_READY)          \
    STATUS(SERVER_STATUS_DATA_INVALID)        \
    STATUS(SERVER_STATUS_INTERNAL_ERROR)      \
    STATUS(SERVER_STATUS_TIMEOUT)             \
    STATUS(SERVER_STATUS_MESSAGE_TOO_BIG)     \
    STATUS(SERVER_STATUS_INVALID_POINTER)

typedef enum
{
    SERVER_STATUSES(GENERATE_ENUM)
} SERVER_STATUS;

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
} message;

/**
 * Waits for a message to be received
 *
 * @param msg received message
 *
 * @returns status of the server
 */
SERVER_STATUS receive_message(message **msg);
/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the server
 */
SERVER_STATUS send_message(const message *msg);
/**
 * Create a message that indicates an successful action
 *
 * @param response created message
 *
 * @returns status of the server
 */
SERVER_STATUS prepare_success_response(message **response);
/**
 * Create a message that indicates an error
 *
 * @param response created message
 *
 * @returns status of the server
 */
SERVER_STATUS prepare_failure_response(message **response);

#endif // IREE_RUNTIME_UTILS_PROTOCOL_H_
