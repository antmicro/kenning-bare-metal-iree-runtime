/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/iree_runtime.c"
#include "../iree-runtime/iree_runtime.h"
#include "mock_model.h"
#include "mock_protocol.h"
#include "mock_uart.h"
#include "unity.h"

#define TEST_CASE(...)

message *gp_message = NULL;
message *gp_message_sent = NULL;
message *gp_message_to_receive = NULL;

const char *const MODEL_STATUS_STR[] = {MODEL_STATUSES(GENERATE_STR)};
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const SERVER_STATUS_STR[] = {SERVER_STATUSES(GENERATE_STR)};

/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 * @param msg prepared message
 */
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message **msg);

/**
 * Mock of receive message protocol function. Writes gp_message to output
 *
 * @param msg received message
 * @param num_calls number of mock calls
 *
 * @returns status of the server
 */
SERVER_STATUS mock_receive_message(message **msg, int num_calls);

/**
 * Mock of send message protocol function. Writes gp_message to output
 *
 * @param msg message to be sent
 * @param num_calls number of mock calls
 *
 * @returns status of the server
 */
SERVER_STATUS mock_send_message(const message *msg, int num_calls);

/**
 * Mock of runtime callback without response
 *
 * @param request incoming message
 */
RUNTIME_STATUS mock_callback_without_response(message **request);

/**
 * Mock of runtime callback with success response
 *
 * @param request incoming message
 */
RUNTIME_STATUS mock_callback_with_ok_response(message **request);

/**
 * Mock of runtime callback with error response
 *
 * @param request incoming message
 */
RUNTIME_STATUS mock_callback_with_error_response(message **request);

/**
 * Mock of runtime callback with success response with payload
 *
 * @param request incoming message
 */
RUNTIME_STATUS mock_callback_with_ok_response_with_payload(message **request);

/**
 * Mock of runtime callback that returns error
 *
 * @param request incoming message
 */
RUNTIME_STATUS mock_callback_error(message **request);

void setUp(void) {}

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

    uart_init_IgnoreAndReturn(UART_STATUS_OK);

    status = init_server();

    TEST_ASSERT_TRUE(status);
}

TEST_CASE(UART_STATUS_INVALID_ARGUMENT_BAUDRATE)
TEST_CASE(UART_STATUS_INVALID_ARGUMENT_STOP_BITS)
TEST_CASE(UART_STATUS_INVALID_ARGUMENT_WORDSIZE)
TEST_CASE(UART_STATUS_INVALID_POINTER)
/**
 * Tests if init server fails when UART init fails
 */
void test_RuntimeInitServerShouldFailIfInitUARTFails(UART_STATUS uart_error)
{
    bool status = true;

    uart_init_IgnoreAndReturn(uart_error);

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
    message *msg = NULL;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_to_receive);
    receive_message_StubWithCallback(mock_receive_message);

    status = wait_for_message(&msg);

    TEST_ASSERT_TRUE(status);
    TEST_ASSERT_EQUAL_PTR(gp_message_to_receive, msg);
}

TEST_CASE(SERVER_STATUS_TIMEOUT)
TEST_CASE(SERVER_STATUS_DATA_INVALID)
TEST_CASE(SERVER_STATUS_CLIENT_DISCONNECTED)
/**
 * Tests if wait for message fails if protocol receive message fails
 */
void test_RuntimeWaitForMessageShouldFailIfProtocolReceiveFails(SERVER_STATUS server_error)
{
    bool status = true;
    message *msg = NULL;

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
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message);

    runtime_status = ok_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, gp_message);
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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = ok_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// error_callback
// ========================================================

/**
 * Tests if error callback has no response
 */
void test_RuntimeErrorCallbackShouldReturnNoResponse(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, &gp_message);

    runtime_status = error_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
    TEST_ASSERT_EQUAL_PTR(NULL, gp_message);
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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = error_callback(&gp_message);

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

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    load_model_input_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                     MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = data_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if data callback fails if model input loading fails
 */
void test_RuntimeDataCallbackShouldFailIfLoadModelInputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    load_model_input_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                     MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = data_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = data_callback(&gp_message);

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

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    load_model_weights_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                       MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = model_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if model callback fails if model weights loading fails
 */
void test_RuntimeModelCallbackShouldFailIfLoadModelWeightsFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    load_model_weights_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                       MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = model_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = model_callback(&gp_message);

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

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    run_model_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if process callback fails if model inference fails
 */
void test_RuntimeProcessCallbackShouldFailIfRunModelFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    run_model_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = process_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = process_callback(&gp_message);

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

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if output callback fails when model output loading fails
 */
void test_RuntimeOutputCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;

    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    get_model_output_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = output_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = output_callback(&gp_message);

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

    prepare_message(MESSAGE_TYPE_STATS, NULL, 0, &gp_message);

    get_statistics_IgnoreAndReturn(MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = stats_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

/**
 * Tests if IO spec callback fails if get statistics fails
 */
void test_RuntimeStatsCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_STATS, data, sizeof(data), &gp_message);

    get_statistics_IgnoreAndReturn(MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = stats_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = stats_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// iospec_callback
// ========================================================
void test_RuntimeIOSpecCallbackShouldRunModel(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    load_model_struct_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                      MODEL_STATUS_OK);
    prepare_success_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = iospec_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_OK, runtime_status);
}

void test_RuntimeIOSpecCallbackShouldFailIfGetModelOutputFails(void)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    uint8_t data[] = "some data";

    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    load_model_struct_ExpectAndReturn(gp_message->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size),
                                      MODEL_STATUS_IREE_ERROR);
    prepare_failure_response_IgnoreAndReturn(SERVER_STATUS_NOTHING);

    runtime_status = iospec_callback(&gp_message);

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

    prepare_message(message_type, NULL, 0, &gp_message);

    runtime_status = iospec_callback(&gp_message);

    TEST_ASSERT_EQUAL_UINT(RUNTIME_STATUS_INVALID_MESSAGE_TYPE, runtime_status);
}

// ========================================================
// mocks
// ========================================================

SERVER_STATUS mock_receive_message(message **msg, int num_calls)
{
    *msg = gp_message_to_receive;

    return SERVER_STATUS_DATA_READY;
}

SERVER_STATUS mock_send_message(const message *msg, int num_calls)
{
    gp_message_sent = malloc(sizeof(message_size_t) + msg->message_size);
    memcpy(gp_message_sent, msg, sizeof(message_size_t) + msg->message_size);

    return SERVER_STATUS_NOTHING;
}

RUNTIME_STATUS mock_callback_without_response(message **request)
{
    *request = NULL;

    return RUNTIME_STATUS_OK;
}

RUNTIME_STATUS mock_callback_with_ok_response(message **request)
{
    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_sent);

    *request = gp_message_sent;

    return RUNTIME_STATUS_OK;
}

RUNTIME_STATUS mock_callback_with_error_response(message **request)
{
    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, &gp_message_sent);

    *request = gp_message_sent;

    return RUNTIME_STATUS_OK;
}

RUNTIME_STATUS mock_callback_with_ok_response_with_payload(message **request)
{
    uint8_t payload[512];
    prepare_message(MESSAGE_TYPE_OK, payload, sizeof(payload), &gp_message_sent);

    *request = gp_message_sent;

    return RUNTIME_STATUS_OK;
}

RUNTIME_STATUS mock_callback_error(message **request)
{
    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, request);

    return RUNTIME_STATUS_ERROR;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message **msg)
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
