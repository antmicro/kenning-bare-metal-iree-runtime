#include "../iree-runtime/utils/protocol.h"
#include "mock_uart.h"
#include "unity.h"

#include <string.h>

message *g_message = NULL;
uint8_t *g_uart_buffer = NULL;

UART_STATUS mock_uart_read(uint8_t *data, size_t data_length, int num_calls);
UART_STATUS mock_uart_write(const uint8_t *data, size_t data_length, int num_calls);

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size);

void setUp(void)
{
    uart_read_StubWithCallback(mock_uart_read);
    uart_write_StubWithCallback(mock_uart_write);
}

void tearDown(void)
{
    if (IS_VALID_POINTER(g_message))
    {
        free(g_message);
        g_message = NULL;
    }
    if (IS_VALID_POINTER(g_uart_buffer))
    {
        free(g_uart_buffer);
        g_uart_buffer = NULL;
    }
}

void test_ProtocolReceiveMessageShouldReadMessageWithoutPayloadFromUART(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_DATA_READY, server_status);
    TEST_ASSERT_EQUAL_UINT(g_message->message_size, msg->message_size);
    TEST_ASSERT_EQUAL_UINT(g_message->message_type, msg->message_type);
}

void test_ProtocolReceiveMessageShouldReadMessageWithPayloadFromUART(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";
    message *msg;

    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_DATA_READY, server_status);
    TEST_ASSERT_EQUAL_UINT(g_message->message_size, msg->message_size);
    TEST_ASSERT_EQUAL_UINT(g_message->message_type, msg->message_type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(message_data, msg->payload, sizeof(message_data));
}

void test_ProtocolReceiveMessageShouldFailWhenMsgPointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));

    server_status = receive_message(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

void test_ProtocolReceiveMessageShouldFailIfMessageTooBig(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    uint8_t message_data[] = "some data";
    message *msg;

    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));
    g_message->message_size = MAX_MESSAGE_SIZE_BYTES + 1;

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_MESSAGE_TOO_BIG, server_status);
}

void test_ProtocolReceiveMessageShouldFailWhenUARTReadError(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_read_IgnoreAndReturn(UART_STATUS_RECEIVE_ERROR);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_CLIENT_DISCONNECTED, server_status);
}

void test_ProtocolReceiveMessageShouldFailWhenUARTReadTimeout(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_read_IgnoreAndReturn(UART_STATUS_TIMEOUT);

    server_status = receive_message(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_TIMEOUT, server_status);
}

void test_ProtocolSendMessageShouldWriteToUART(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    server_status = send_message(g_message);

    message test;
    message *msg_from_buffer = (message *)g_uart_buffer;

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(g_message->message_size, msg_from_buffer->message_size);
    TEST_ASSERT_EQUAL_UINT(g_message->message_type, msg_from_buffer->message_type);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(g_message->payload, msg_from_buffer->payload, g_message->message_size);
}

void test_ProtocolSendMessageShouldFailIfMessagePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = send_message(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

void test_ProtocolSendMessageShouldFailIfUARTWriteFails(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);
    uart_write_IgnoreAndReturn(UART_STATUS_TIMEOUT);

    server_status = send_message(g_message);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_TIMEOUT, server_status);
}

void test_ProtocolPrepareSuccessResponseShouldPrepareEmptyOKMessage(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    server_status = prepare_success_response(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_OK, msg->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), msg->message_size);
}

void test_ProtocolPrepareSuccessResponseShouldFailIfThePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = prepare_success_response(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

void test_ProtocolPrepareFailureResponseShouldPrepareEmptyERRORMessage(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;
    message *msg;

    server_status = prepare_failure_response(&msg);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_NOTHING, server_status);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_ERROR, msg->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), msg->message_size);
}

void test_ProtocolPrepareFailureResponseShouldFailfThePointerIsInvalid(void)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = prepare_failure_response(NULL);

    TEST_ASSERT_EQUAL_UINT(SERVER_STATUS_INVALID_POINTER, server_status);
}

UART_STATUS mock_uart_read(uint8_t *data, size_t data_length, int num_calls)
{
    num_calls %= 3;

    switch (num_calls)
    {
    case 0:
        memcpy(data, &g_message->message_size, sizeof(message_size_t));
        break;
    case 1:
        memcpy(data, &g_message->message_type, sizeof(message_type_t));
        break;
    case 2:
        memcpy(data, &g_message->payload, g_message->message_size);
        break;
    }

    return UART_STATUS_OK;
}

UART_STATUS mock_uart_write(const uint8_t *data, size_t data_length, int num_calls)
{
    if (IS_VALID_POINTER(g_uart_buffer))
    {
        free(g_uart_buffer);
        g_uart_buffer = NULL;
    }

    g_uart_buffer = malloc(data_length);
    memcpy(g_uart_buffer, data, data_length);

    return UART_STATUS_OK;
}

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(g_message))
    {
        free(g_message);
        g_message = NULL;
    }
    g_message = malloc(sizeof(message) + payload_size);
    g_message->message_size = sizeof(message_type_t) + payload_size;
    g_message->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy(g_message->payload, payload, payload_size);
    }
}
