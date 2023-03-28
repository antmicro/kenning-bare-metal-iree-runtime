#ifndef MOCK_IREE_WRAPPER_H_
#define MOCK_IREE_WRAPPER_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum
{
    IREE_HAL_ELEMENT_TYPE_INT_8,
    IREE_HAL_ELEMENT_TYPE_UINT_8,
    IREE_HAL_ELEMENT_TYPE_INT_16,
    IREE_HAL_ELEMENT_TYPE_UINT_16,
    IREE_HAL_ELEMENT_TYPE_INT_32,
    IREE_HAL_ELEMENT_TYPE_UINT_32,
    IREE_HAL_ELEMENT_TYPE_INT_64,
    IREE_HAL_ELEMENT_TYPE_UINT_64,
    IREE_HAL_ELEMENT_TYPE_FLOAT_16,
    IREE_HAL_ELEMENT_TYPE_FLOAT_32,
    IREE_HAL_ELEMENT_TYPE_FLOAT_64,
} iree_hal_element_types_t;

#define IREE_HAL_ELEMENT_TYPES(HAL_ELEMENT_TYPE)            \
    HAL_ELEMENT_TYPE("i8", IREE_HAL_ELEMENT_TYPE_INT_8)     \
    HAL_ELEMENT_TYPE("u8", IREE_HAL_ELEMENT_TYPE_UINT_8)    \
    HAL_ELEMENT_TYPE("i16", IREE_HAL_ELEMENT_TYPE_INT_16)   \
    HAL_ELEMENT_TYPE("u16", IREE_HAL_ELEMENT_TYPE_UINT_16)  \
    HAL_ELEMENT_TYPE("i32", IREE_HAL_ELEMENT_TYPE_INT_32)   \
    HAL_ELEMENT_TYPE("u32", IREE_HAL_ELEMENT_TYPE_UINT_32)  \
    HAL_ELEMENT_TYPE("i64", IREE_HAL_ELEMENT_TYPE_INT_64)   \
    HAL_ELEMENT_TYPE("u64", IREE_HAL_ELEMENT_TYPE_UINT_64)  \
    HAL_ELEMENT_TYPE("f16", IREE_HAL_ELEMENT_TYPE_FLOAT_16) \
    HAL_ELEMENT_TYPE("f32", IREE_HAL_ELEMENT_TYPE_FLOAT_32) \
    HAL_ELEMENT_TYPE("f64", IREE_HAL_ELEMENT_TYPE_FLOAT_64)

typedef enum
{
    IREE_WRAPPER_STATUS_OK,
    IREE_WRAPPER_STATUS_ERROR,
} IREE_WRAPPER_STATUS;

/**
 * Model struct constraints
 */
#define MAX_MODEL_INPUT_NUM 2
#define MAX_MODEL_INPUT_DIM 4
#define MAX_MODEL_OUTPUTS 12
#define MAX_LENGTH_ENTRY_FUNC_NAME 20
#define MAX_LENGTH_MODEL_NAME 20

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
    iree_hal_element_types_t hal_element_type;
    uint8_t entry_func[MAX_LENGTH_ENTRY_FUNC_NAME];
    uint8_t model_name[MAX_LENGTH_MODEL_NAME];
} MlModel;

#define BREAK_ON_IREE_ERROR(status) \
    if (iree_status)                \
    {                               \
        break;                      \
    }

#define CHECK_IREE_STATUS(status)           \
    if (status)                             \
    {                                       \
        return (IREE_WRAPPER_STATUS)status; \
    }

IREE_WRAPPER_STATUS create_context(const uint8_t *model_data, const size_t model_data_size);
IREE_WRAPPER_STATUS prepare_input_buffer(const MlModel *model_struct, const uint8_t *model_input);
IREE_WRAPPER_STATUS prepare_output_buffer();
IREE_WRAPPER_STATUS run_inference();
IREE_WRAPPER_STATUS get_output(uint8_t *model_output);
IREE_WRAPPER_STATUS get_model_stats(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                    size_t *statistics_size);
void release_input_buffer();
void release_output_buffer();

#endif // MOCK_IREE_WRAPPER_H_
