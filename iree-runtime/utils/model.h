#ifndef IREE_RUNTIME_UTIL_MODEL_H_
#define IREE_RUNTIME_UTIL_MODEL_H_

#include "utils.h"
#include <stdint.h>
#include <string.h>

#include "iree/hal/drivers/local_sync/sync_device.h"
#include "iree/hal/local/loaders/vmvx_module_loader.h"
#include "iree/modules/hal/module.h"
#include "iree/vm/bytecode_module.h"
#include "springbok.h"

/**
 * Model struct constraints
 */
#define MAX_MODEL_INPUT_NUM 2
#define MAX_MODEL_INPUT_DIM 4
#define MAX_MODEL_OUTPUTS 12
#define MAX_LENGTH_ENTRY_FUNC_NAME 20
#define MAX_LENGTH_MODEL_NAME 20

#define CHECK_IREE_STATUS(status)        \
    if (!iree_status_is_ok(iree_status)) \
    {                                    \
        return MODEL_STATUS_IREE_ERROR;  \
    }

/**
 * An enum that describes server status
 */
#define MODEL_STATUSES(STATUS)            \
    STATUS(MODEL_STATUS_OK)               \
    STATUS(MODEL_STATUS_INVALID_ARGUMENT) \
    STATUS(MODEL_STATUS_INVALID_STATE)    \
    STATUS(MODEL_STATUS_IREE_ERROR)

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

#define IREE_HAL_ELEMENT_TYPES(X)            \
    X("i8", IREE_HAL_ELEMENT_TYPE_INT_8)     \
    X("u8", IREE_HAL_ELEMENT_TYPE_UINT_8)    \
    X("i16", IREE_HAL_ELEMENT_TYPE_INT_16)   \
    X("u16", IREE_HAL_ELEMENT_TYPE_UINT_16)  \
    X("i32", IREE_HAL_ELEMENT_TYPE_INT_32)   \
    X("u32", IREE_HAL_ELEMENT_TYPE_UINT_32)  \
    X("i64", IREE_HAL_ELEMENT_TYPE_INT_64)   \
    X("u64", IREE_HAL_ELEMENT_TYPE_UINT_64)  \
    X("f16", IREE_HAL_ELEMENT_TYPE_FLOAT_16) \
    X("f32", IREE_HAL_ELEMENT_TYPE_FLOAT_32) \
    X("f64", IREE_HAL_ELEMENT_TYPE_FLOAT_64)

/**
 * A struct that contains model parameters
 */
typedef struct __attribute__((packed))
{
    uint32_t num_input;
    uint32_t num_input_dim[MAX_MODEL_INPUT_NUM];
    uint32_t input_shape[MAX_MODEL_INPUT_NUM][MAX_MODEL_INPUT_DIM];
    uint32_t input_length[MAX_MODEL_INPUT_NUM];
    uint32_t input_size_bytes[MAX_MODEL_INPUT_NUM];
    uint32_t num_output;
    uint32_t output_length[MAX_MODEL_OUTPUTS];
    uint32_t output_size_bytes;
    enum iree_hal_element_types_t hal_element_type;
    uint8_t entry_func[MAX_LENGTH_ENTRY_FUNC_NAME];
    uint8_t model_name[MAX_LENGTH_MODEL_NAME];
} MlModel;

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

MODEL_STATUS get_statistics(iree_hal_allocator_statistics_t *statistics);

#endif // IREE_RUNTIME_UTIL_MODEL_H_
