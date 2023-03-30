#include "../iree-runtime/iree_runtime.h"
#include "mock_model.h"
#include "mock_protocol.h"
#include "mock_uart.h"
#include "unity.h"

#define TEST_CASE(...)

message *g_message = NULL;

const char *const MODEL_STATUS_STR[] = {MODEL_STATUSES(GENERATE_STR)};
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const SERVER_STATUS_STR[] = {SERVER_STATUSES(GENERATE_STR)};

/**
 * Prepares message of given type and payload and store result in g_message
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 */
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

/**
 * Tests if ok callback has no response
 */
void test_RuntimeOKCallbackShouldReturnNoReponse(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    runtime_status = ok_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, g_message);
}

/**
 * Tests if ok callback fails for invalid pointer
 */
void test_RuntimeOKCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = ok_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if ok callback fails for request message type
 */
void test_RuntimeOkCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = ok_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// error_callback
// ========================================================

/**
 * Tests if error callback has no response
 */
void test_RuntimeErrorCallbackShouldReturnNoReponse(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0);

    runtime_status = error_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, g_message);
}

/**
 * Tests if error callback fails for invalid pointer
 */
void test_RuntimeErrorCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = error_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if error callback fails for request message type
 */
void test_RuntimeErrorCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = error_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// data_callback
// ========================================================

/**
 * Tests if data callback properly loads model input
 */
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

/**
 * Tests if data callback fails if model input loading fails
 */
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

/**
 * Tests if data callback fails for invalid pointer
 */
void test_RuntimeDataCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = data_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if data callback fails for request message type
 */
void test_RuntimeDataCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = data_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// model_callback
// ========================================================

/**
 * Tests if model callback properly loads model weights
 */
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

/**
 * Tests if model callback fails if model weights loading fails
 */
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

/**
 * Tests if model callback fails for invalid pointer
 */
void test_RuntimeModelCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = model_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if model callback fails for request message type
 */
void test_RuntimeModelCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = model_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// process_callback
// ========================================================

/**
 * Tests if process callback properly runs model inference
 */
void test_RuntimeProcessCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0);

    run_model_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if process callback fails if model inference fails
 */
void test_RuntimeProcessCallbackShouldFailIfRunModelFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0);

    run_model_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

/**
 * Tests if process callback fails for invalid pointer
 */
void test_RuntimeProcessCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = process_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if process callback fails for request message type
 */
void test_RuntimeProcessCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = process_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// output_callback
// ========================================================

/**
 * Tests if output callback properly loads model output
 */
void test_RuntimeOutputCallbackShouldLoadModelOutput(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if output callback fails when model output loading fails
 */
void test_RuntimeOutputCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_MODEL_ERROR, runtime_status);
}

/**
 * Tests if output callback fails for invalid pointer
 */
void test_RuntimeOutputCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = output_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if output callback fails for request message type
 */
void test_RuntimeOutputCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

    runtime_status = output_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// stats_callback
// ========================================================

/**
 * Tests if IO spec callback properly loads execution statistics
 */
void test_RuntimeStatsCallbackShouldLoadStats(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_STATS, NULL, 0);

    get_statistics_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = stats_callback(&g_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if IO spec callback fails if get statistics fails
 */
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

/**
 * Tests if stats callback fails for invalid pointer
 */
void test_RuntimeStatsCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = stats_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if stats callback fails for request message type
 */
void test_RuntimeStatsCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

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

/**
 * Tests if IO spec callback fails for invalid pointer
 */
void test_RuntimeIOSpecCallbackShouldFailForInvalidPointer(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    runtime_status = iospec_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_POINTER, runtime_status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
/**
 * Tests if IO spec callback fails for request message type
 */
void test_RuntimeIOSpecCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(message_type, NULL, 0);

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
