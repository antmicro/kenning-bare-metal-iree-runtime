/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "protocol.h"

GENERATE_MODULE_STATUSES_STR(PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

static uint8_t __attribute__((aligned(4))) g_message_buffer[MAX_MESSAGE_SIZE_BYTES + 2];

/**
 * Returns pointer to a message buffer with payload aligned to 4 bytes
 *
 * @returns pointer to a message buffer
 */
static message_t *get_message_buffer()
{
    // payload should be aligned to 4 bytes and as header (msg size and msg type) are
    // total 6 bytes, we need to shift msg buffer pointer 2 bytes
    return (message_t *)(g_message_buffer + 2);
}

status_t receive_message(message_t **msg)
{
    status_t status = STATUS_OK;
    message_size_t msg_size = 0;
    MESSAGE_TYPE msg_type = MESSAGE_TYPE_OK;
    uint8_t data[4];

    VALIDATE_POINTER(msg, PROTOCOL_STATUS_INV_PTR);

    // read size of the message
    status = uart_read(data, sizeof(message_size_t));
    CHECK_UART_STATUS(status);

    msg_size = *((message_size_t *)data);

    if (msg_size > MAX_MESSAGE_SIZE_BYTES)
    {
        return PROTOCOL_STATUS_MSG_TOO_BIG;
    }

    // read type of the message
    status = uart_read(data, sizeof(message_type_t));
    CHECK_UART_STATUS(status);
    msg_type = *((message_type_t *)data);

    // get pointer to the message buffer
    *msg = get_message_buffer();
    VALIDATE_POINTER(*msg, PROTOCOL_STATUS_INV_PTR);

    (*msg)->message_size = msg_size;
    (*msg)->message_type = msg_type;

    // read the payload
    status = uart_read((*msg)->payload, MESSAGE_SIZE_PAYLOAD(msg_size));
    if (STATUS_OK != status)
    {
        *msg = NULL;
    }
    CHECK_UART_STATUS(status);

    return PROTOCOL_STATUS_DATA_READY;
}

status_t send_message(const message_t *msg)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(msg, PROTOCOL_STATUS_INV_PTR);

    status = uart_write((uint8_t *)msg, MESSAGE_SIZE_FULL(msg->message_size));

    CHECK_UART_STATUS(status);

    return status;
}

status_t prepare_success_response(message_t **response)
{
    VALIDATE_POINTER(response, PROTOCOL_STATUS_INV_PTR);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return PROTOCOL_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_OK;
    return STATUS_OK;
}
status_t prepare_failure_response(message_t **response)
{
    VALIDATE_POINTER(response, PROTOCOL_STATUS_INV_PTR);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return PROTOCOL_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_ERROR;
    return STATUS_OK;
}
