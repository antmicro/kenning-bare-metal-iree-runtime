#include "iree_runtime.h"

const char *RUNTIME_STATUS_STR[] = {RUNTIME_STATUSES(GENERATE_STR)};
extern const char *MESSAGE_TYPE_STR[];
extern const char *MODEL_STATUS_STR[];

/**
 * Main Runtime function. It initializes UART and then handles messages in an infinite loop.
 */
int main()
{
    int ret = 0;
    UART_STATUS uart_status = UART_OK;
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    uart_config config = {.data_bits = 8, .stop_bits = 1, .parity = false, .baudrate = 115200};
    uart_status = uart_init(&config);
    if (UART_OK != uart_status)
    {
        LOG_ERROR("UART error");
        return 1;
    }
    LOG_INFO("UART initialized");
    LOG_INFO("Runtime started");
    // main runtime loop
    while (1)
    {
        message *msg = NULL;
        server_status = receive_message(&msg);
        if (SERVER_STATUS_TIMEOUT == server_status)
        {
            LOG_WARN("Receive message timeout");
            continue;
        }
        if (SERVER_STATUS_DATA_READY != server_status)
        {
            LOG_ERROR("Error receiving message: %d", server_status);
            continue;
        }
        LOG_INFO("Received message. Size: %d, type: %d (%s)", msg->message_size, msg->message_type,
                 MESSAGE_TYPE_STR[msg->message_type]);
        runtime_status = msg_callback[msg->message_type](&msg);
        if (RUNTIME_STATUS_OK != runtime_status)
        {
            LOG_ERROR("Runtime error: %d (%s)", runtime_status, RUNTIME_STATUS_STR[runtime_status]);
        }
        if (NULL != msg)
        {
            LOG_INFO("Sending reponse. Size: %d, type: %d (%s)", msg->message_size, msg->message_type,
                     MESSAGE_TYPE_STR[msg->message_type]);
            server_status = send_message(msg);
            if (SERVER_STATUS_NOTHING != server_status)
            {
                LOG_ERROR("Error sending message: %d", server_status);
            }
        }
    }
    return ret;
}

/**
 * Handles OK message
 *
 * @param request incoming message. It is overwritten with NULL as there is no response
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS ok_callback(message **request)
{
    LOG_WARN("Unexpected message received: MESSAGE_TYPE_OK");
    *request = NULL;
    return RUNTIME_STATUS_OK;
}

/**
 * Handles ERROR message
 *
 * @param request incoming message. It is overwritten with NULL as there is no response
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS error_callback(message **request)
{
    LOG_WARN("Unexpected message received: MESSAGE_TYPE_ERROR");
    *request = NULL;
    return RUNTIME_STATUS_OK;
}

/**
 * Handles DATA message that contains model input. It calls model's function that loads it.
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS data_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;

    m_status = load_model_input((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("load_model_input returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }
    s_status = prepare_success_response(request);
    RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);

    return RUNTIME_STATUS_OK;
}

/**
 * Handles MODEL message that contains model data. It calls model's function that loads the model.
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS model_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;

    m_status = load_model_weights((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("load_model_weights returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }
    s_status = prepare_success_response(request);
    RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);

    return RUNTIME_STATUS_OK;
}

/**
 * Handles PROCESS message. It calls model's function that runs it
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS process_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;

    m_status = run_model();

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("run_model returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }
    s_status = prepare_success_response(request);
    RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
    return RUNTIME_STATUS_OK;
}

/**
 * Handles OUTPUT message. It retrieves model inference output and sends it back
 *
 * @param request incoming message. It is overwritten by the response message (DATA message containig model output or
 *                ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS output_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;
    size_t model_output_size = 0;

    m_status = get_model_output(MAX_MESSAGE_SIZE_BYTES - sizeof(message), (*request)->payload, &model_output_size);

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("get_model_output returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }
    (*request)->message_size = model_output_size + sizeof(((message *)0)->message_type);

    return RUNTIME_STATUS_OK;
}

/**
 * Handles STATS message. It retrieves model statistics
 *
 * @param request incoming message. It is overwritten by the response message (STATS message containig model
 *                statistics or ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS stats_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;

    m_status = get_statistics((iree_hal_allocator_statistics_t *)&(*request)->payload);

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("get_statistics returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }

    (*request)->message_size = sizeof(iree_hal_allocator_statistics_t) + sizeof(((message *)0)->message_type);

    return RUNTIME_STATUS_OK;
}

/**
 * Handles IOSPEC message. It loads model IO specification
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the runtime
 */
RUNTIME_STATUS iospec_callback(message **request)
{
    MODEL_STATUS m_status = MODEL_STATUS_OK;
    SERVER_STATUS s_status = SERVER_STATUS_NOTHING;

    m_status = load_model_struct((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    if (MODEL_STATUS_OK != m_status)
    {
        LOG_ERROR("load_model_struct returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);
        s_status = prepare_failure_response(request);
        RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
        return RUNTIME_STATUS_MODEL_ERROR;
    }
    s_status = prepare_success_response(request);
    RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);
    return RUNTIME_STATUS_OK;
}