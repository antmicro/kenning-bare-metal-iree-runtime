#ifndef IREE_RUNTIME_UTIL_MODEL_H_
#define IREE_RUNTIME_UTIL_MODEL_H_

#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef __UNIT_TEST__
#include "iree_wrapper.h"
#include "springbok.h"
#else // __UNIT_TEST__
#include "mocks/iree_wrapper.h"
#include "mocks/springbok.h"
#endif // __UNIT_TEST__

#define CHECK_IREE_WRAPPER_STATUS(status) \
    if (IREE_WRAPPER_STATUS_OK != status) \
    {                                     \
        return MODEL_STATUS_IREE_ERROR;   \
    }

/**
 * An enum that describes server status
 */
#define MODEL_STATUSES(STATUS)            \
    STATUS(MODEL_STATUS_OK)               \
    STATUS(MODEL_STATUS_INVALID_ARGUMENT) \
    STATUS(MODEL_STATUS_INVALID_STATE)    \
    STATUS(MODEL_STATUS_IREE_ERROR)       \
    STATUS(MODEL_STATUS_INVALID_POINTER)

typedef enum
{
    MODEL_STATUSES(GENERATE_ENUM)
} MODEL_STATUS;

/**
 * An enum that describes model state
 */
typedef enum
{
    MODEL_STATE_UNINITIALIZED = 0,
    MODEL_STATE_STRUCT_LOADED = 1,
    MODEL_STATE_WEIGHTS_LOADED = 2,
    MODEL_STATE_INPUT_LOADED = 3,
    MODEL_STATE_INFERENCE_DONE = 4,
} MODEL_STATE;

/**
 * Loads model struct from given buffer
 *
 * @param model_struct_data buffer that contains model struct
 * @param data_size size of the buffer
 *
 * @returns status of the model
 */
MODEL_STATUS load_model_struct(const uint8_t *model_struct_data, const size_t data_size);

/**
 * Loads model weights from given buffer
 *
 * @param model_data buffer that contains model weights
 * @param model_data_size size of the buffer
 *
 * @returns status of the model
 */
MODEL_STATUS load_model_weights(const uint8_t *model_data, const size_t model_data_size);

/**
 * Loads model input from given buffer
 *
 * @param model_input buffer that contains model input
 * @param model_input_size size of the buffer
 *
 * @returns status of the model
 */
MODEL_STATUS load_model_input(const uint8_t *model_input, const size_t model_input_size);

/**
 * Runs model inference
 *
 * @returns status of the model
 */
MODEL_STATUS run_model();

/**
 * Writes model output to given buffer
 *
 * @param buffer_size size of the buffer
 * @param model_output buffer to save model output
 * @param model_output_size actual size of the saved data
 *
 * @returns status of the model
 */
MODEL_STATUS get_model_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size);

/**
 * Retrieves model statistics
 *
 * @returns status of the model
 */
MODEL_STATUS get_statistics(uint8_t *statistics, size_t *statistics_size);

#endif // IREE_RUNTIME_UTIL_MODEL_H_
