/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/iree_runtime.c"
#include "../iree-runtime/iree_runtime.h"
#include "mock_i2c.h"
#include "mock_model.h"
#include "mock_protocol.h"
#include "mock_sensor.h"
#include "mock_uart.h"
#include "mock_utils.h"
#include "mocks/sensor_mock.h"
#include "unity.h"

#define TEST_CASE(...)

message_t *gp_message = NULL;
message_t *gp_message_sent = NULL;
message_t *gp_message_to_receive = NULL;

GENERATE_MODULE_STATUSES_STR(MODEL);
GENERATE_MODULE_STATUSES_STR(PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

#define SENSOR_DEFAULT_READ_DELAY (0.0001f)

/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 * @param msg prepared message
 */
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg);

/**
 * Mock of get status str
 *
 * @param status status code
 *
 * @returns status name as string
 */
const char *mock_get_status_str(status_t status, int num_calls);

/**
 * Mock of receive message protocol function. Writes gp_message to output
 *
 * @param msg received message
 * @param num_calls number of mock calls
 *
 * @returns status of the server
 */
status_t mock_receive_message(message_t **msg, int num_calls);

/**
 * Mock of send message protocol function. Writes gp_message to output
 *
 * @param msg message to be sent
 * @param num_calls number of mock calls
 *
 * @returns status of the server
 */
status_t mock_send_message(const message_t *msg, int num_calls);

/**
 * Mock of runtime callback without response
 *
 * @param request incoming message
 */
status_t mock_callback_without_response(message_t **request);

/**
 * Mock of runtime callback with success response
 *
 * @param request incoming message
 */
status_t mock_callback_with_ok_response(message_t **request);

/**
 * Mock of runtime callback with error response
 *
 * @param request incoming message
 */
status_t mock_callback_with_error_response(message_t **request);

/**
 * Mock of runtime callback with success response with payload
 *
 * @param request incoming message
 */
status_t mock_callback_with_ok_response_with_payload(message_t **request);

/**
 * Mock of runtime callback that returns error
 *
 * @param request incoming message
 */
status_t mock_callback_error(message_t **request);

void setUp(void) { get_status_str_StubWithCallback(mock_get_status_str); }

void tearDown(void)
{
    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
        gp_message = NULL;
    }
    if (IS_VALID_POINTER(gp_message_sent))
    {
        free(gp_message_sent);
        gp_message_sent = NULL;
    }
    if (IS_VALID_POINTER(gp_message_to_receive))
    {
        free(gp_message_to_receive);
        gp_message_to_receive = NULL;
    }
}

// ========================================================
// init_server
// ========================================================

/**
 * Tests if init server initializes UART
 */
void test_RuntimeInitServerShouldInitUART()
{
    bool status = true;

    uart_init_IgnoreAndReturn(STATUS_OK);
    i2c_init_IgnoreAndReturn(STATUS_OK);
    sensor_init_IgnoreAndReturn(STATUS_OK);

    status = init_server();

    TEST_ASSERT_TRUE(status);
}

TEST_CASE(UART_STATUS_INV_ARG_BAUDRATE)
TEST_CASE(UART_STATUS_INV_ARG_STOP_BITS)
TEST_CASE(UART_STATUS_INV_ARG_WORDSIZE)
TEST_CASE(UART_STATUS_INV_PTR)
/**
 * Tests if init server fails when UART init fails
 */
void test_RuntimeInitServerShouldFailIfInitUARTFails(status_t uart_error)
{
    bool status = true;

    uart_init_IgnoreAndReturn(uart_error);
    i2c_init_IgnoreAndReturn(STATUS_OK);
    sensor_init_IgnoreAndReturn(STATUS_OK);

    status = init_server();

    TEST_ASSERT_FALSE(status);
}

TEST_CASE(I2C_STATUS_INV_PTR)
TEST_CASE(I2C_STATUS_INV_ARG)
/**
 * Tests if init server fails when I2C init fails
 */
void test_RuntimeInitServerShouldFailIfInitI2CFails(status_t i2c_error)
{
    bool status = true;

    uart_init_IgnoreAndReturn(STATUS_OK);
    i2c_init_IgnoreAndReturn(i2c_error);
    sensor_init_IgnoreAndReturn(STATUS_OK);

    status = init_server();

    TEST_ASSERT_FALSE(status);
}

TEST_CASE(SENSOR_STATUS_NO_SENSOR_AVAILABLE)
TEST_CASE(SENSOR_STATUS_INV_SENSOR)
/**
 * Tests if init server fails when I2C init fails
 */
void test_RuntimeInitServerShouldFailIfInitSensorFails(status_t sensor_error)
{
    bool status = true;

    uart_init_IgnoreAndReturn(STATUS_OK);
    i2c_init_IgnoreAndReturn(STATUS_OK);
    sensor_init_IgnoreAndReturn(sensor_error);

    status = init_server();

    TEST_ASSERT_FALSE(status);
}

// ========================================================
// wait_for_message
// ========================================================

/**
 * Tests if wait for message properly receives message from protocol
 */
void test_RuntimeWaitForMessageShouldGetMessageFromProtocol()
{
    bool status = true;
    message_t *msg = NULL;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_to_receive);
    receive_message_StubWithCallback(mock_receive_message);

    status = wait_for_message(&msg);

    TEST_ASSERT_TRUE(status);
    TEST_ASSERT_EQUAL_PTR(gp_message_to_receive, msg);
}

TEST_CASE(PROTOCOL_STATUS_TIMEOUT)
TEST_CASE(PROTOCOL_STATUS_DATA_INV)
TEST_CASE(PROTOCOL_STATUS_CLIENT_DISCONNECTED)
/**
 * Tests if wait for message fails if protocol receive message fails
 */
void test_RuntimeWaitForMessageShouldFailIfProtocolReceiveFails(status_t server_error)
{
    bool status = true;
    message_t *msg = NULL;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_to_receive);
    receive_message_ExpectAndReturn(&msg, server_error);

    status = wait_for_message(&msg);

    TEST_ASSERT_FALSE(status);
    TEST_ASSERT_EQUAL_PTR(NULL, msg);
}

// ========================================================
// handle_message
// ========================================================

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
/**
 * Tests if handle message calls proper callback for messages without response
 */
void test_RuntimeHandleMessageShouldCallProperCallbackWithoutResponseForMessage(MESSAGE_TYPE message_type)
{
    prepare_message(message_type, NULL, 0, &gp_message);

    g_msg_callback[message_type] = mock_callback_without_response;

    handle_message(gp_message);

    TEST_ASSERT_EQUAL_PTR(NULL, gp_message_sent);
}

TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if handle message calls proper callback for messages with success response without payload
 */
void test_RuntimeHandleMessageShouldCallProperCallbackWithOKResponseWithoutPayloadForMessage(MESSAGE_TYPE message_type)
{
    prepare_message(message_type, NULL, 0, &gp_message);
    g_msg_callback[message_type] = mock_callback_with_ok_response;
    send_message_StubWithCallback(mock_send_message);

    handle_message(gp_message);

    TEST_ASSERT_NOT_EQUAL_INT(NULL, gp_message_sent);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_OK, gp_message_sent->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), gp_message_sent->message_size);
}

TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if handle message calls proper callback for messages with failure response without payload
 */
void test_RuntimeHandleMessageShouldCallProperCallbackWithErrorResponseWithoutPayloadForMessage(
    MESSAGE_TYPE message_type)
{
    prepare_message(message_type, NULL, 0, &gp_message);
    g_msg_callback[message_type] = mock_callback_with_error_response;
    send_message_StubWithCallback(mock_send_message);

    handle_message(gp_message);

    TEST_ASSERT_NOT_EQUAL_INT(NULL, gp_message_sent);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_ERROR, gp_message_sent->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), gp_message_sent->message_size);
}

TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
/**
 * Tests if handle message calls proper callback for messages with success response with payload
 */
void test_RuntimeHandleMessageShouldCallProperCallbackWithOKResponseWithPayloadForMessage(MESSAGE_TYPE message_type)
{
    prepare_message(message_type, NULL, 0, &gp_message);
    g_msg_callback[message_type] = mock_callback_with_ok_response_with_payload;
    send_message_StubWithCallback(mock_send_message);

    handle_message(gp_message);

    TEST_ASSERT_NOT_EQUAL_INT(NULL, gp_message_sent);
    TEST_ASSERT_EQUAL_UINT(MESSAGE_TYPE_OK, gp_message_sent->message_type);
    TEST_ASSERT_GREATER_THAN_UINT(sizeof(message_type_t), gp_message_sent->message_size);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if handle message properly sends error message when callback fails
 */
void test_RuntimeHandleMessageShouldSendErrorIfCallbackFails(MESSAGE_TYPE message_type)
{
    prepare_message(message_type, NULL, 0, &gp_message);

    g_msg_callback[message_type] = mock_callback_error;
    send_message_StubWithCallback(mock_send_message);

    handle_message(gp_message);

    TEST_ASSERT_NOT_EQUAL_INT(NULL, gp_message_sent);
    TEST_ASSERT_EQUAL_PTR(MESSAGE_TYPE_ERROR, gp_message_sent->message_type);
    TEST_ASSERT_EQUAL_UINT(sizeof(message_type_t), gp_message_sent->message_size);
}

/**
 * Tests if handle message does nothing when message pointer is invalid
 */
void test_RuntimeHandleMessageShouldReturnForInvalidPointer(void)
{
    handle_message(NULL);

    // send_message and callback not called
}

// ========================================================
// ok_callback
// ========================================================

/**
 * Tests if ok callback has no response
 */
void test_RuntimeOKCallbackShouldReturnNoResponse(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message);

    status = ok_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_PTR(NULL, gp_message);
}

/**
 * Tests if ok callback fails for invalid pointer
 */
void test_RuntimeOKCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = ok_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if ok callback fails for invalid request message type
 */
void test_RuntimeOkCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = ok_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// error_callback
// ========================================================

/**
 * Tests if error callback has no response
 */
void test_RuntimeErrorCallbackShouldReturnNoResponse(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, &gp_message);

    status = error_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_PTR(NULL, gp_message);
}

/**
 * Tests if error callback fails for invalid pointer
 */
void test_RuntimeErrorCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = error_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if error callback fails for ivalid request message type
 */
void test_RuntimeErrorCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = error_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// data_callback
// ========================================================

/**
 * Tests if data callback properly loads model input
 */
void test_RuntimeDataCallbackShouldLoadDataToModelInput(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    load_model_input_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size), STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = data_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if data callback fails if model input loading fails
 */
void test_RuntimeDataCallbackShouldFailIfLoadModelInputFails(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    load_model_input_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                     MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = data_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if data callback fails for invalid pointer
 */
void test_RuntimeDataCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = data_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if data callback fails for ivalid request message type
 */
void test_RuntimeDataCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = data_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// model_callback
// ========================================================

/**
 * Tests if model callback properly loads model weights
 */
void test_RuntimeModelCallbackShouldLoadModelWeights(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    load_model_weights_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size), STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = model_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if model callback fails if model weights loading fails
 */
void test_RuntimeModelCallbackShouldFailIfLoadModelWeightsFails(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    load_model_weights_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                       MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = model_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if model callback fails for invalid pointer
 */
void test_RuntimeModelCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = model_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if model callback fails for invalid request message type
 */
void test_RuntimeModelCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = model_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// process_callback
// ========================================================

/**
 * Tests if process callback properly runs model inference
 */
void test_RuntimeProcessCallbackShouldRunModel(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    run_model_IgnoreAndReturn(STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = process_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if process callback fails if model inference fails
 */
void test_RuntimeProcessCallbackShouldFailIfRunModelFails(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    run_model_IgnoreAndReturn(MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = process_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if process callback fails for invalid pointer
 */
void test_RuntimeProcessCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = process_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if process callback fails for invalid request message type
 */
void test_RuntimeProcessCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = process_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// output_callback
// ========================================================

/**
 * Tests if output callback properly loads model output
 */
void test_RuntimeOutputCallbackShouldLoadModelOutput(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    get_model_output_IgnoreAndReturn(STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = output_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if output callback fails when model output loading fails
 */
void test_RuntimeOutputCallbackShouldFailIfGetModelOutputFails(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = output_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if output callback fails for invalid pointer
 */
void test_RuntimeOutputCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = output_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_STATS)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if output callback fails for invalid request message type
 */
void test_RuntimeOutputCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = output_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// stats_callback
// ========================================================

/**
 * Tests if stats callback properly loads execution statistics
 */
void test_RuntimeStatsCallbackShouldLoadStats(void)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_STATS, NULL, 0, &gp_message);

    get_statistics_IgnoreAndReturn(STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = stats_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if stats callback fails if get statistics fails
 */
void test_RuntimeStatsCallbackShouldFailIfGetModelOutputFails(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_STATS, data, sizeof(data), &gp_message);

    get_statistics_IgnoreAndReturn(MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = stats_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if stats callback fails for invalid pointer
 */
void test_RuntimeStatsCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = stats_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_IOSPEC)
/**
 * Tests if stats callback fails for invalid request message type
 */
void test_RuntimeStatsCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = stats_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// iospec_callback
// ========================================================

/**
 * Tests if IO spec callback load model struct
 */
void test_RuntimeIOSpecCallbackShouldLoadModelStruct(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    load_model_struct_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size), STATUS_OK);
    prepare_success_response_IgnoreAndReturn(STATUS_OK);

    status = iospec_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
}

/**
 * Tests if IO spec callback fails when struct loading fails
 */
void test_RuntimeIOSpecCallbackShouldFailIfStructLoadingFails(void)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    load_model_struct_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                      MODEL_STATUS_INV_STATE);
    prepare_failure_response_IgnoreAndReturn(STATUS_OK);

    status = iospec_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if IO spec callback fails for invalid pointer
 */
void test_RuntimeIOSpecCallbackShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = iospec_callback(NULL);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_PTR, status);
}

TEST_CASE(MESSAGE_TYPE_OK)
TEST_CASE(MESSAGE_TYPE_ERROR)
TEST_CASE(MESSAGE_TYPE_DATA)
TEST_CASE(MESSAGE_TYPE_MODEL)
TEST_CASE(MESSAGE_TYPE_PROCESS)
TEST_CASE(MESSAGE_TYPE_OUTPUT)
TEST_CASE(MESSAGE_TYPE_STATS)
/**
 * Tests if IO spec callback fails for invalid request message type
 */
void test_RuntimeIOSpecCallbackShouldFailForInvalidMessageType(MESSAGE_TYPE message_type)
{
    status_t status = STATUS_OK;

    prepare_message(message_type, NULL, 0, &gp_message);

    status = iospec_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INV_MSG_TYPE, status);
}

// ========================================================
// mocks
// ========================================================

const char *mock_get_status_str(status_t status, int num_calls) { return "STATUS_STR"; }

status_t mock_receive_message(message_t **msg, int num_calls)
{
    *msg = gp_message_to_receive;

    return PROTOCOL_STATUS_DATA_READY;
}

status_t mock_send_message(const message_t *msg, int num_calls)
{
    gp_message_sent = malloc(sizeof(message_size_t) + msg->message_size);
    memcpy(gp_message_sent, msg, sizeof(message_size_t) + msg->message_size);

    return STATUS_OK;
}

status_t mock_callback_without_response(message_t **request)
{
    *request = NULL;

    return STATUS_OK;
}

status_t mock_callback_with_ok_response(message_t **request)
{
    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_sent);

    *request = gp_message_sent;

    return STATUS_OK;
}

status_t mock_callback_with_error_response(message_t **request)
{
    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, &gp_message_sent);

    *request = gp_message_sent;

    return STATUS_OK;
}

status_t mock_callback_with_ok_response_with_payload(message_t **request)
{
    uint8_t payload[512];
    prepare_message(MESSAGE_TYPE_OK, payload, sizeof(payload), &gp_message_sent);

    *request = gp_message_sent;

    return STATUS_OK;
}

status_t mock_callback_error(message_t **request)
{
    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, request);

    return RUNTIME_STATUS_ERROR;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg)
{
    if (IS_VALID_POINTER(*msg))
    {
        free(*msg);
        *msg = NULL;
    }
    *msg = malloc(sizeof(message_size_t) + payload_size);
    (*msg)->message_size = sizeof(message_type_t) + payload_size;
    (*msg)->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy((*msg)->payload, payload, payload_size);
    }
}
