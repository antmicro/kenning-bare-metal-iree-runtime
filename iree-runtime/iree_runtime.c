#include "iree_runtime.h"

const char *const RUNTIME_STATUS_STR[] = {RUNTIME_STATUSES(GENERATE_STR)};
extern const char *const MESSAGE_TYPE_STR[];
extern const char *const MODEL_STATUS_STR[];
extern const char *const SERVER_STATUS_STR[];

callback_ptr msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS
#undef ENTRY
};

static bool init_server();
static bool wait_for_message(message **msg);
static void handle_message(message *msg);

#ifndef __UNIT_TEST__
/**
 * Main Runtime function. It initializes UART and then handles messages in an infinite loop.
 */
int main()
{
    int ret = 0;
    if (!init_server())
    {
        LOG_ERROR("Init server failed");
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
    return ret;
}
#endif // __UNIT_TEST__

static bool init_server()
{
    UART_STATUS uart_status = UART_STATUS_OK;

    uart_config config = {.data_bits = 8, .stop_bits = 1, .parity = false, .baudrate = 115200};
    uart_status = uart_init(&config);
    if (UART_STATUS_OK != uart_status)
    {
        LOG_ERROR("UART error");
        return false;
    }
    LOG_INFO("UART initialized");
    LOG_INFO("Runtime started");
    return true;
}

static bool wait_for_message(message **msg)
{
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

    server_status = receive_message(msg);
    if (SERVER_STATUS_TIMEOUT == server_status)
    {
        LOG_WARN("Receive message timeout");
        return false;
    }
    if (SERVER_STATUS_DATA_READY != server_status)
    {
        LOG_ERROR("Error receiving message: %d (%s)", server_status, SERVER_STATUS_STR[server_status]);
        return false;
    }
    LOG_INFO("Received message. Size: %d, type: %d (%s)", (*msg)->message_size, (*msg)->message_type,
             MESSAGE_TYPE_STR[(*msg)->message_type]);

    return true;
}

static void handle_message(message *msg)
{
    RUNTIME_STATUS runtime_status = RUNTIME_STATUS_OK;
    SERVER_STATUS server_status = SERVER_STATUS_NOTHING;

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
            LOG_ERROR("Error sending message: %d (%s)", server_status, SERVER_STATUS_STR[server_status]);
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
RUNTIME_STATUS ok_callback(message **request)
{
    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_OK != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }
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
    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_ERROR != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }
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

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_DATA != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }

    m_status = load_model_input((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(m_status, request, "load_model_input returned %d (%s)", m_status,
                           MODEL_STATUS_STR[m_status]);

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

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_MODEL != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }
    m_status = load_model_weights((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(m_status, request, "load_model_weights returned %d (%s)", m_status,
                           MODEL_STATUS_STR[m_status]);

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

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_PROCESS != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }

    m_status = run_model();

    CHECK_MODEL_STATUS_LOG(m_status, request, "run_model returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);

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
    size_t model_output_size = 0;

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_OUTPUT != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }

    m_status = get_model_output(MAX_MESSAGE_SIZE_BYTES - sizeof(message), (*request)->payload, &model_output_size);

    CHECK_MODEL_STATUS_LOG(m_status, request, "get_model_output returned %d (%s)", m_status,
                           MODEL_STATUS_STR[m_status]);

    (*request)->message_size = model_output_size + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

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
    size_t statistics_length = 0;

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_STATS != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }

    m_status = get_statistics((uint8_t *)&(*request)->payload, &statistics_length);

    CHECK_MODEL_STATUS_LOG(m_status, request, "get_statistics returned %d (%s)", m_status, MODEL_STATUS_STR[m_status]);

    (*request)->message_size = statistics_length + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

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

    if (!IS_VALID_POINTER(request))
    {
        return RUNTIME_STATUS_INVALID_POINTER;
    }
    if (MESSAGE_TYPE_IOSPEC != (*request)->message_type)
    {
        return RUNTIME_STATUS_INVALID_MESSAGE_TYPE;
    }
    m_status = load_model_struct((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_MODEL_STATUS_LOG(m_status, request, "load_model_struct returned %d (%s)", m_status,
                           MODEL_STATUS_STR[m_status]);

    s_status = prepare_success_response(request);
    RETURN_ON_ERROR(s_status, RUNTIME_STATUS_SERVER_ERROR);

    return RUNTIME_STATUS_OK;
}
