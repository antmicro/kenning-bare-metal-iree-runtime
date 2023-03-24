#include "../iree-runtime/iree_runtime.h"
#include "mock_model.h"
#include "mock_protocol.h"
#include "mock_springbok.h"
#include "unity.h"

message *g_message = NULL;

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size);

void setUp(void) {}

void tearDown(void)
{
    if (IS_VALID_POINTER(g_message))
    {
        free(g_message);
        g_message = NULL;
    }
}

void test_RuntimeOKCallbackShouldReturnNoReponse(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = ok_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, g_message);
}

void test_RuntimeOKCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = ok_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeOkCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0);

    runtime_status = ok_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

void test_RuntimeErrorCallbackShouldReturnNoReponse(void)
{
    RUNTIME_STATUS runtime_status RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0);

    runtime_status = error_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, g_message);
}

void test_RuntimeErrorCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = error_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeErrorCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = error_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

void test_RuntimeDataCallbackShouldLoadDataToModelInput(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data));

    runtime_status = data_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), g_message->message_size);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_OK, g_message->message_type);
}

void test_RuntimeDataCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = data_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeDataCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = data_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(g_message))
    {
        free(g_message);
        g_message = NULL:
    }
    g_message = malloc(sizeof(message) + payload_size);
    g_message->message_size = sizeof(message_size_t) + payload_size);
    g_message->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy(g_message->payload, payload, payload_size);
    }
}
