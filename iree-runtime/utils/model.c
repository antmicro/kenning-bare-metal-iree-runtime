#include "model.h"

const char *const MODEL_STATUS_STR[] = {MODEL_STATUSES(GENERATE_STR)};

MlModel g_model_struct;

#ifndef __UNIT_TEST__
static MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;
#else  // __UNIT_TEST__
MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;
#endif // __UNIT_TEST__

MODEL_STATUS load_model_struct(const uint8_t *model_struct_data, const size_t data_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;

    if (!IS_VALID_POINTER(model_struct_data))
    {
        return MODEL_STATUS_INVALID_POINTER;
    }
    if (sizeof(MlModel) != data_size)
    {
        LOG_ERROR("Wrong model struct size: %d. Should be: %d.", data_size, sizeof(MlModel));
        return MODEL_STATUS_INVALID_ARGUMENT;
    }

    g_model_struct = *((MlModel *)model_struct_data);

    // validate struct
    if (g_model_struct.num_input > MAX_MODEL_INPUT_NUM || g_model_struct.num_output > MAX_MODEL_OUTPUTS)
    {
        return MODEL_STATUS_INVALID_ARGUMENT;
    }
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        if (g_model_struct.num_input_dim[i] > MAX_MODEL_INPUT_DIM)
        {
            return MODEL_STATUS_INVALID_ARGUMENT;
        }
    }

    char *dtype = (char *)&g_model_struct.hal_element_type;

    // this x-macro retrieves string label and HAL element enum value from IREE_HAL_ELEMENT_TYPES table and
    // compares this label with the string received with the struct. If label is equal to this string then the
    // relevand enum value is assigned. If none of the labels is equal to this string then the final else is
    // hit and error is returned
#define CHECK_HAL_ELEM_TYPE(label, element_type)        \
    if (0 == strncmp(dtype, label, 4))                  \
    {                                                   \
        g_model_struct.hal_element_type = element_type; \
    }                                                   \
    else
    IREE_HAL_ELEMENT_TYPES(CHECK_HAL_ELEM_TYPE)
    {
        LOG_ERROR("Wrong dtype %s", dtype);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }
#undef CHECK_HAL_ELEM_TYPE

    LOG_INFO("Loaded model struct. Model name: %s", g_model_struct.model_name);

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    return status;
}

MODEL_STATUS load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    IREE_WRAPPER_STATUS iree_status = IREE_WRAPPER_STATUS_OK;

    if (!IS_VALID_POINTER(model_weights_data))
    {
        return MODEL_STATUS_INVALID_POINTER;
    }
    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    // free input/output resources
    release_output_buffer();
    release_input_buffer();

    iree_status = create_context(model_weights_data, data_size);
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    LOG_INFO("Loaded model weights");

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    return status;
}

MODEL_STATUS load_model_input(const uint8_t *model_input, const size_t model_input_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    IREE_WRAPPER_STATUS iree_status = IREE_WRAPPER_STATUS_OK;

    if (!IS_VALID_POINTER(model_input))
    {
        return MODEL_STATUS_INVALID_POINTER;
    }
    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    // validate size of received data
    size_t expected_size = 0;
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        expected_size += g_model_struct.input_length[i] * g_model_struct.input_size_bytes[i];
    }
    if (model_input_size != expected_size)
    {
        LOG_ERROR("Invalid model input size: %d. Expected size: %d", model_input_size, expected_size);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }

    // free resources
    release_input_buffer();

    // setup buffers for inputs
    iree_status = prepare_input_buffer(&g_model_struct, model_input);
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    LOG_INFO("Loaded model input");

    g_model_state = MODEL_STATE_INPUT_LOADED;

    return status;
}

MODEL_STATUS run_model()
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    IREE_WRAPPER_STATUS iree_status = IREE_WRAPPER_STATUS_OK;

    if (g_model_state < MODEL_STATE_INPUT_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    // free resources
    release_output_buffer();

    // setup buffers for outputs
    iree_status = prepare_output_buffer();
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    // perform inference
    iree_status = run_inference();
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    LOG_INFO("Model inference done");

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    return status;
}

MODEL_STATUS get_model_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    IREE_WRAPPER_STATUS iree_status = IREE_WRAPPER_STATUS_OK;

    if (!IS_VALID_POINTER(model_output) || !IS_VALID_POINTER(model_output_size))
    {
        return MODEL_STATUS_INVALID_POINTER;
    }
    if (g_model_state < MODEL_STATE_INFERENCE_DONE)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    size_t output_size = 0;
    for (int i = 0; i < g_model_struct.num_output; ++i)
    {
        output_size += g_model_struct.output_length[i] * g_model_struct.output_size_bytes;
    }
    if (buffer_size < output_size)
    {
        LOG_ERROR("Buffer is too small. Buffer size: %d. Model output size: %d", buffer_size, output_size);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }
    *model_output_size = output_size;

    iree_status = get_output(model_output);
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    LOG_INFO("Model output retrieved");

    return status;
}

MODEL_STATUS get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer, size_t *statistics_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    IREE_WRAPPER_STATUS iree_status = IREE_WRAPPER_STATUS_OK;

    if (!IS_VALID_POINTER(statistics_buffer) || !IS_VALID_POINTER(statistics_size))
    {
        return MODEL_STATUS_INVALID_POINTER;
    }
    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    iree_status = get_model_stats(statistics_buffer_size, statistics_buffer, statistics_size);
    CHECK_IREE_WRAPPER_STATUS(iree_status);

    LOG_INFO("Model statistics retrieved");

    return status;
}
