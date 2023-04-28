/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/protocol.h"
#include "mock_uart.h"
#include "unity.h"

#include <string.h>

#define TEST_CASE(...)

message *gp_message = NULL;
uint8_t *gp_uart_buffer = NULL;

/**
 * Mocks UART read function
 *
 * @param data buffer for results
 * @param data_length length of the buffer
 * @param num_calls numver of mock calls
 *
 * @returns status of read action
 */
UART_STATUS mock_uart_read(uint8_t *data, size_t data_length, int num_calls);

/**
 * Mocks UART write function
 *
 * @param data buffer to be written
 * @param data_length length of the buffer
 * @param num_calls numver of mock calls
 *
 * @returns status of read action
 */
UART_STATUS mock_uart_write(const uint8_t *data, size_t data_length, int num_calls);

/**
 * Prepares message of given type and payload and store result in gp_message
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 */
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size);

void setUp(void)
{
    uart_read_StubWithCallback(mock_uart_read);
    uart_write_StubWithCallback(mock_uart_write);
}

void tearDown(void)
{
    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
        gp_message = NULL;
    }
    if (IS_VALID_POINTER(gp_uart_buffer))
    {
        free(gp_uart_buffer);
        gp_uart_buffer = NULL;
    }
}

// ========================================================
// receive_message
// ========================================================

TEST_CASE(0) // MESSAGE_TYPE_OK
TEST_CASE(1) // MESSAGE_TYPE_ERROR
TEST_CASE(4) // MESSAGE_TYPE_PROCESS
TEST_CASE(5) // MESSAGE_TYPE_OUTPUT
/**
 * Tests if protocol receive message reads message without payload properly from UART
 */
void test_ProtocolReceiveMessageShouldReadMessageWithoutPayloadFromUART(uint32_t message_type)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(message_type, NULL, 0);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_DATA_READY, server_status);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_size, msg->message_size);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_type, msg->message_type);
}

TEST_CASE(2) // MESSAGE_TYPE_DATA
TEST_CASE(3) // MESSAGE_TYPE_MODEL
TEST_CASE(6) // MESSAGE_TYPE_STATS
TEST_CASE(7) // MESSAGE_TYPE_IOSPEC
/**
 * Tests if protocol receive message reads message with payload properly from UART
 */
void test_ProtocolReceiveMessageShouldReadMessageWithPayloadFromUART(uint32_t message_type)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";
    message *msg;

    prepare_message(message_type, message_data, sizeof(message_data));

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_DATA_READY, server_status);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_size, msg->message_size);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_type, msg->message_type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(message_data, msg->payload, sizeof(message_data));
}

/**
 * Tests if protocol receive message fails for invalid pointer
 */
void test_ProtocolReceiveMessageShouldFailWhenMsgPointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));

    server_status = receive_message(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

/**
 * Tests if protocol receive message fails if message size is too big
 */
void test_ProtocolReceiveMessageShouldFailIfMessageTooBig(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";
    message *msg;

    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));
    gp_message->message_size = MAX_MESSAGE_SIZE_BYTES + 1;

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_MESSAGE_TOO_BIG, server_status);
}

/**
 * Tests if protocol receive message fails if UART read fails
 */
void test_ProtocolReceiveMessageShouldFailWhenUARTReadError(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_read_IgnoreAndReturn(UART_STATUS_RECEIVE_ERROR);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_CLIENT_DISCONNECTED, server_status);
}

/**
 * Tests if protocol receive message hits timeout if UART read does so
 */
void test_ProtocolReceiveMessageShouldFailWhenUARTReadTimeout(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_read_IgnoreAndReturn(UART_STATUS_TIMEOUT);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_TIMEOUT, server_status);
}

// ========================================================
// send_message
// ========================================================

TEST_CASE(0) // MESSAGE_TYPE_OK
TEST_CASE(1) // MESSAGE_TYPE_ERROR
TEST_CASE(4) // MESSAGE_TYPE_PROCESS
TEST_CASE(5) // MESSAGE_TYPE_OUTPUT
/**
 * Tests if protocol send message writes properly message without payload to UART
 */
void test_ProtocolSendMessageShouldWriteMessageWithourPayloadToUART(uint32_t message_type)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    prepare_message(message_type, NULL, 0);

    server_status = send_message(gp_message);

    message test;
    message *msg_from_buffer = (message *)gp_uart_buffer;

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_size, msg_from_buffer->message_size);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_type, msg_from_buffer->message_type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gp_message->payload, msg_from_buffer->payload, gp_message->message_size);
}

TEST_CASE(2) // MESSAGE_TYPE_DATA
TEST_CASE(3) // MESSAGE_TYPE_MODEL
TEST_CASE(6) // MESSAGE_TYPE_STATS
TEST_CASE(7) // MESSAGE_TYPE_IOSPEC
/**
 * Tests if protocol send message writes properly message with payload to UART
 */
void test_ProtocolSendMessageShouldWriteMessageWithPayloadToUART(uint32_t message_type)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_payload[128];

    prepare_message(message_type, message_payload, sizeof(message_payload));

    server_status = send_message(gp_message);

    message test;
    message *msg_from_buffer = (message *)gp_uart_buffer;

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_size, msg_from_buffer->message_size);
    TEST_ASSERT_EQUAL_UINT(gp_message->message_type, msg_from_buffer->message_type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gp_message->payload, msg_from_buffer->payload, gp_message->message_size);
}

/**
 * Tests if protocol send message fails if message pointer is invalid
 */
void test_ProtocolSendMessageShouldFailIfMessagePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = send_message(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

/**
 * Tests if protocol send message fails if UART write fails
 */
void test_ProtocolSendMessageShouldFailIfUARTWriteFails(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_write_IgnoreAndReturn(UART_STATUS_TIMEOUT);

    server_status = send_message(gp_message);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_TIMEOUT, server_status);
}

// ========================================================
// prepare_success_response
// ========================================================

/**
 * Tests if protocol prepare success message properly creates empty OK message
 */
void test_ProtocolPrepareSuccessResponseShouldPrepareEmptyOKMessage(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    server_status = prepare_success_response(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_OK, msg->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), msg->message_size);
}

/**
 * Tests if protocol prepare success message fails if message pointer is invalid
 */
void test_ProtocolPrepareSuccessResponseShouldFailIfThePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = prepare_success_response(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

// ========================================================
// prepare_failure_response
// ========================================================

/**
 * Tests if protocol prepare failure message properly creates empty ERROR message
 */
void test_ProtocolPrepareFailureResponseShouldPrepareEmptyERRORMessage(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    server_status = prepare_failure_response(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_ERROR, msg->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), msg->message_size);
}

/**
 * Tests if protocol prepare failure message fails if message pointer is invalid
 */
void test_ProtocolPrepareFailureResponseShouldFailfThePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = prepare_failure_response(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

// ========================================================
// mocks
// ========================================================

UART_STATUS mock_uart_read(uint8_t *data, size_t data_length, int num_calls)
{
    static size_t data_read = 0;

    switch (data_read)
    {
    case 0:
        memcpy(data, &gp_message->message_size, sizeof(message_size_t));
        data_read += data_length;
        break;
    case sizeof(message_size_t):
        memcpy(data, &gp_message->message_type, sizeof(message_type_t));
        data_read += data_length;
        break;
    case sizeof(message):
        memcpy(data, &gp_message->payload, gp_message->message_size);
        data_read = 0;
        break;
    default:
        return UART_STATUS_RECEIVE_ERROR;
    }

    return UART_STATUS_OK;
}

UART_STATUS mock_uart_write(const uint8_t *data, size_t data_length, int num_calls)
{
    if (IS_VALID_POINTER(gp_uart_buffer))
    {
        free(gp_uart_buffer);
        gp_uart_buffer = NULL;
    }

    gp_uart_buffer = malloc(data_length);
    memcpy(gp_uart_buffer, data, data_length);

    return UART_STATUS_OK;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
        gp_message = NULL;
    }
    gp_message = malloc(sizeof(message) + payload_size);
    gp_message->message_size = sizeof(message_type_t) + payload_size;
    gp_message->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy(gp_message->payload, payload, payload_size);
    }
}
