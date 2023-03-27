#include "../iree-runtime/iree_runtime.h"
#include "mock_model.h"
#include "mock_protocol.h"
#include "mock_uart.h"
#include "unity.h"

message *g_message = NULL;

const char *const MODEL_STATUS_STR[] = {MODEL_STATUSES(GENERATE_STR)};
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const SERVER_STATUS_STR[] = {SERVER_STATUSES(GENERATE_STR)};

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

// ========================================================
// ok_callback
// ========================================================
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

// ========================================================
// error_callback
// ========================================================
void test_RuntimeErrorCallbackShouldReturnNoReponse(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

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

// ========================================================
// data_callback
// ========================================================
void test_RuntimeDataCallbackShouldLoadDataToModelInput(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data));

    load_model_input_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                     MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = data_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeDataCallbackShouldFailIfLoadModelInputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data));

    load_model_input_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                     MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = data_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
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

// ========================================================
// model_callback
// ========================================================
void test_RuntimeModelCallbackShouldLoadModelWeights(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data));

    load_model_weights_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                       MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = model_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeModelCallbackShouldFailIfLoadModelWeightsFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data));

    load_model_weights_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                       MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = model_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

void test_RuntimeModelCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = model_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeModelCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = model_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// process_callback
// ========================================================
void test_RuntimeProcessCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0);

    run_model_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeProcessCallbackShouldFailIfRunModelFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0);

    run_model_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

void test_RuntimeProcessCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = process_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeProcessCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// output_callback
// ========================================================
void test_RuntimeOutputCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeOutputCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

void test_RuntimeOutputCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = output_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeOutputCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// stats_callback
// ========================================================
void test_RuntimeStatsCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_STATS, NULL, 0);

    get_statistics_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = stats_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeStatsCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_STATS, data, sizeof(data));

    get_statistics_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = stats_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

void test_RuntimeStatsCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = stats_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeStatsCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = stats_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// iospec_callback
// ========================================================
void test_RuntimeIOSpecCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data));

    load_model_struct_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                      MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = iospec_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeIOSpecCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data));

    load_model_struct_ExpectAndReturn(g_message->payload, MESSAGE_SIZE_PAYLOAD(g_message->message_size),
                                      MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = iospec_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

void test_RuntimeIOSpecCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = iospec_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

void test_RuntimeIOSpecCallbackShouldFailForInvalidMessageType(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = iospec_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// helper functions
// ========================================================
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(g_message))
    {
        free(g_message);
        g_message = NULL;
    }
    g_message = malloc(sizeof(message) + payload_size);
    g_message->message_size = sizeof(message_size_t) + payload_size;
    g_message->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy(g_message->payload, payload, payload_size);
    }
}
