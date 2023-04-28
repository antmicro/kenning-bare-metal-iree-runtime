/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "protocol.h"

const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const SERVER_STATUS_STR[] = {SERVER_STATUSES(GENERATE_STR)};

static uint8_t __attribute__((aligned(4))) g_message_buffer[MAX_MESSAGE_SIZE_BYTES + 2];

/**
 * Returns pointer to a message buffer with payload aligned to 4 bytes
 *
 * @returns pointer to a message buffer
 */
static message *get_message_buffer()
{
    // payload should be alligned to 4 bytes and as header (msg size and msg type) are
    // total 6 bytes, we need to shift msg buffer pointer 2 bytes
    return (message *)(g_message_buffer + 2);
}

SERVER_STATUS receive_message(message **msg)
{
    UART_STATUS status = UART_STATUS_OK;
    message_size_t msg_size = 0;
    MESSAGE_TYPE msg_type = MESSAGE_TYPE_OK;
    uint8_t data[4];

    VALIDATE_POINTER(msg, SERVER_STATUS_INVALID_POINTER);

    // read size of the message
    status = uart_read(data, sizeof(message_size_t));
    CHECK_UART_STATUS(status);

    msg_size = *((message_size_t *)data);

    if (msg_size > MAX_MESSAGE_SIZE_BYTES)
    {
        return SERVER_STATUS_MESSAGE_TOO_BIG;
    }

    // read type of the message
    status = uart_read(data, sizeof(message_type_t));
    CHECK_UART_STATUS(status);
    msg_type = *((message_type_t *)data);

    // get pointer to the message buffer
    *msg = get_message_buffer();
    VALIDATE_POINTER(*msg, SERVER_STATUS_INVALID_POINTER);

    (*msg)->message_size = msg_size;
    (*msg)->message_type = msg_type;

    // read the payload
    status = uart_read((*msg)->payload, MESSAGE_SIZE_PAYLOAD(msg_size));
    if (UART_STATUS_OK != status)
    {
        *msg = NULL;
    }
    CHECK_UART_STATUS(status);

    return SERVER_STATUS_DATA_READY;
}

SERVER_STATUS send_message(const message *msg)
{
    UART_STATUS status = UART_STATUS_OK;

    VALIDATE_POINTER(msg, SERVER_STATUS_INVALID_POINTER);

    status = uart_write((uint8_t *)msg, MESSAGE_SIZE_FULL(msg->message_size));

    CHECK_UART_STATUS(status);

    return SERVER_STATUS_NOTHING;
}

SERVER_STATUS prepare_success_response(message **response)
{
    VALIDATE_POINTER(response, SERVER_STATUS_INVALID_POINTER);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return SERVER_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_OK;
    return SERVER_STATUS_NOTHING;
}
SERVER_STATUS prepare_failure_response(message **response)
{
    VALIDATE_POINTER(response, SERVER_STATUS_INVALID_POINTER);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return SERVER_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_ERROR;
    return SERVER_STATUS_NOTHING;
}
