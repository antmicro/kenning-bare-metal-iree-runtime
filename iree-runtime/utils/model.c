/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "model.h"

GENERATE_MODULE_STATUSES_STR(MODEL);

MlModel g_model_struct;

ut_static MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;

MODEL_STATE get_model_state() { return g_model_state; }

void reset_model_state() { g_model_state = MODEL_STATE_UNINITIALIZED; }

status_t load_model_struct(const uint8_t *model_struct_data, const size_t data_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(model_struct_data, MODEL_STATUS_INV_PTR);

    if (sizeof(MlModel) != data_size)
    {
        LOG_ERROR("Wrong model struct size: %d. Should be: %d.", data_size, sizeof(MlModel));
        return MODEL_STATUS_INV_ARG;
    }

    g_model_struct = *((MlModel *)model_struct_data);

    // validate struct
    if (g_model_struct.num_input < 1 || g_model_struct.num_input > MAX_MODEL_INPUT_NUM ||
        g_model_struct.num_output < 1 || g_model_struct.num_output > MAX_MODEL_OUTPUTS)
    {
        return MODEL_STATUS_INV_ARG;
    }
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        if (g_model_struct.num_input_dim[i] > MAX_MODEL_INPUT_DIM)
        {
            return MODEL_STATUS_INV_ARG;
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
        LOG_ERROR("Wrong dtype %.4s", dtype);
        return MODEL_STATUS_INV_ARG;
    }
#undef CHECK_HAL_ELEM_TYPE

    LOG_DEBUG("Loaded model struct. Model name: %s", g_model_struct.model_name);

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    return status;
}

status_t load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(model_weights_data, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // free input/output resources
    release_output_buffer();
    release_input_buffer();

    status = create_context(model_weights_data, data_size);
    RETURN_ON_ERROR(status, status);

    LOG_DEBUG("Loaded model weights");

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    return STATUS_OK;
}

status_t get_model_input_size(size_t *model_input_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(model_input_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // compute input size
    size_t size = 0;
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        size += g_model_struct.input_length[i] * g_model_struct.input_size_bytes[i];
    }

    *model_input_size = size;

    return status;
}

status_t load_model_input(const uint8_t *model_input, const size_t model_input_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(model_input, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // validate size of received data
    size_t expected_size = 0;
    status = get_model_input_size(&expected_size);
    RETURN_ON_ERROR(status, status);

    if (model_input_size != expected_size)
    {
        LOG_ERROR("Invalid model input size: %d. Expected size: %d", model_input_size, expected_size);
        return MODEL_STATUS_INV_ARG;
    }

    // free resources
    release_input_buffer();

    // setup buffers for inputs
    status = prepare_input_buffer(&g_model_struct, model_input);
    RETURN_ON_ERROR(status, status);

    LOG_DEBUG("Loaded model input");

    g_model_state = MODEL_STATE_INPUT_LOADED;

    return status;
}

status_t run_model()
{
    status_t status = STATUS_OK;

    if (g_model_state < MODEL_STATE_INPUT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // free resources
    release_output_buffer();

    // setup buffers for outputs
    status = prepare_output_buffer();
    RETURN_ON_ERROR(status, status);

    // perform inference
    status = run_inference();
    RETURN_ON_ERROR(status, status);

    LOG_DEBUG("Model inference done");

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    return status;
}

status_t get_model_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(model_output, MODEL_STATUS_INV_PTR);
    VALIDATE_POINTER(model_output_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_INFERENCE_DONE)
    {
        return MODEL_STATUS_INV_STATE;
    }

    size_t output_size = 0;
    for (int i = 0; i < g_model_struct.num_output; ++i)
    {
        output_size += g_model_struct.output_length[i] * g_model_struct.output_size_bytes;
    }
    if (buffer_size < output_size)
    {
        LOG_ERROR("Buffer is too small. Buffer size: %d. Model output size: %d", buffer_size, output_size);
        return MODEL_STATUS_INV_ARG;
    }
    *model_output_size = output_size;

    status = get_output(model_output);
    RETURN_ON_ERROR(status, status);

    LOG_DEBUG("Model output retrieved");

    return status;
}

status_t get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer, size_t *statistics_size)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(statistics_buffer, MODEL_STATUS_INV_PTR);
    VALIDATE_POINTER(statistics_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    status = get_model_stats(statistics_buffer_size, statistics_buffer, statistics_size);
    RETURN_ON_ERROR(status, status);

    LOG_DEBUG("Model statistics retrieved");

    return status;
}
