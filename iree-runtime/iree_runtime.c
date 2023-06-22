/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iree_runtime.h"

GENERATE_MODULE_STATUSES_STR(RUNTIME);

extern const char *const MESSAGE_TYPE_STR[];

ut_static callback_ptr g_msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS
#undef ENTRY
};

/**
 * Initializes runtime server
 *
 * @returns true if severs is initialized successfully
 */
static bool init_server();

/**
 * Waits for incoming message
 *
 * @param msg pointer to the message
 *
 * @returns true if message is received, false otherwise
 */
static bool wait_for_message(message **msg);

/**
 * Handles received message
 *
 * @param msg pointer to the message
 */
static void handle_message(message *msg);

#ifndef __UNIT_TEST__
/**
 * Main Runtime function. It initializes UART and then handles messages in an infinite loop.
 */
int main()
{
    if (!init_server())
    {
        LOG_ERROR("Server init failed");
        return 1;
    }
    // main runtime loop
    while (1)
    {
        message *msg = NULL;
        if (wait_for_message(&msg))
        {
            handle_message(msg);
        }
    }
    return 0;
}
#endif // __UNIT_TEST__

bool init_server()
{
    status_t status = STATUS_OK;

    uart_config config = {.data_bits = 8, .stop_bits = 1, .parity = false, .baudrate = 115200};
    status = uart_init(&config);
    if (STATUS_OK != status)
    {
        LOG_ERROR("uart_init returned 0x%x (%s)", status, get_status_str(status));
        return false;
    }
    LOG_INFO("UART initialized");

#ifdef __I2C__
    i2c_config_t i2c_config = {.clock_period_nanos = 41};
    status = i2c_init(&i2c_config);
    if (STATUS_OK != status)
    {
        LOG_ERROR("i2c_init returned 0x%x (%s)", status, get_status_str(status));
        return false;
    }
    LOG_INFO("I2C initialized");

#endif // __I2C__

    LOG_INFO("Runtime started");
    return true;
}

bool wait_for_message(message **msg)
{
    status_t status = STATUS_OK;

    if (!IS_VALID_POINTER(msg))
    {
        return false;
    }

    status = receive_message(msg);
    if (PROTOCOL_STATUS_TIMEOUT == status)
    {
        LOG_WARN("Receive message timeout");
        return false;
    }
    if (PROTOCOL_STATUS_DATA_READY != status)
    {
        LOG_ERROR("Error receiving message: %d (%s)", status, get_status_str(status));
        return false;
    }
    LOG_DEBUG("Received message. Size: %d, type: %d (%s)", (*msg)->message_size, (*msg)->message_type,
              MESSAGE_TYPE_STR[(*msg)->message_type]);

    return true;
}

void handle_message(message *msg)
{
    status_t status = STATUS_OK;

    if (!IS_VALID_POINTER(msg))
    {
        return;
    }

    status = g_msg_callback[msg->message_type](&msg);
    if (STATUS_OK != status)
    {
        LOG_ERROR("Runtime error: 0x%x (%s)", status, get_status_str(status));
    }
    if (NULL != msg)
    {
        LOG_DEBUG("Sending reponse. Size: %d, type: %d (%s)", msg->message_size, msg->message_type,
                  MESSAGE_TYPE_STR[msg->message_type]);
        status = send_message(msg);
        if (STATUS_OK != status)
        {
            LOG_ERROR("Error sending message: 0x%x (%s)", status, get_status_str(status));
        }
    }
}
/**
 * Handles OK message
 *
 * @param request incoming message. It is overwritten with NULL as there is no response
 *
 * @returns error status of the runtime
 */
status_t ok_callback(message **request)
{
    VALIDATE_REQUEST(MESSAGE_TYPE_OK, request);

    LOG_WARN("Unexpected message received: MESSAGE_TYPE_OK");
    *request = NULL;
    return STATUS_OK;
}

/**
 * Handles ERROR message
 *
 * @param request incoming message. It is overwritten with NULL as there is no response
 *
 * @returns error status of the runtime
 */
status_t error_callback(message **request)
{
    VALIDATE_REQUEST(MESSAGE_TYPE_ERROR, request);

    LOG_WARN("Unexpected message received: MESSAGE_TYPE_ERROR");
    *request = NULL;
    return STATUS_OK;
}

/**
 * Handles DATA message that contains model input. It calls model's function that loads it.
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
status_t data_callback(message **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_DATA, request);

    status = load_model_input((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(status, request, "load_model_input returned 0x%x (%s)", status, get_status_str(status));

    status = prepare_success_response(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles MODEL message that contains model data. It calls model's function that loads the model.
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
status_t model_callback(message **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_MODEL, request);

    status = load_model_weights((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(status, request, "load_model_weights returned 0x%x (%s)", status, get_status_str(status));

    status = prepare_success_response(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles PROCESS message. It calls model's function that runs it
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
status_t process_callback(message **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_PROCESS, request);

    status = run_model();

    CHECK_MODEL_STATUS_LOG(status, request, "run_model returned 0x%x (%s)", status, get_status_str(status));

    status = prepare_success_response(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles OUTPUT message. It retrieves model inference output and sends it back
 *
 * @param request incoming message. It is overwritten by the response message (DATA message containig model output or
 *                ERROR message)
 *
 * @returns error status of the runtime
 */
status_t output_callback(message **request)
{
    status_t status = STATUS_OK;
    size_t model_output_size = 0;

    VALIDATE_REQUEST(MESSAGE_TYPE_OUTPUT, request);

    status = get_model_output(MAX_MESSAGE_SIZE_BYTES - sizeof(message), (*request)->payload, &model_output_size);

    CHECK_MODEL_STATUS_LOG(status, request, "get_model_output returned 0x%x (%s)", status, get_status_str(status));

    (*request)->message_size = model_output_size + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

    return STATUS_OK;
}

/**
 * Handles STATS message. It retrieves model statistics
 *
 * @param request incoming message. It is overwritten by the response message (STATS message containig model
 *                statistics or ERROR message)
 *
 * @returns error status of the runtime
 */
status_t stats_callback(message **request)
{
    status_t status = STATUS_OK;
    size_t statistics_length = 0;

    VALIDATE_REQUEST(MESSAGE_TYPE_STATS, request);

    status =
        get_statistics(MAX_MESSAGE_SIZE_BYTES - sizeof(message), (uint8_t *)&(*request)->payload, &statistics_length);

    CHECK_MODEL_STATUS_LOG(status, request, "get_statistics returned 0x%x (%s)", status, get_status_str(status));

    (*request)->message_size = statistics_length + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

    return STATUS_OK;
}

/**
 * Handles IOSPEC message. It loads model IO specification
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
status_t iospec_callback(message **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_IOSPEC, request);

    status = load_model_struct((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(status, request, "load_model_struct returned 0x%x (%s)", status, get_status_str(status));

    status = prepare_success_response(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}
