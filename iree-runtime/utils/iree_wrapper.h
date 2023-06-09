/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTIL_IREE_WRAPPER_H_
#define IREE_RUNTIME_UTIL_IREE_WRAPPER_H_

#include "utils.h"

#ifndef __UNIT_TEST__
#include "iree/hal/drivers/local_sync/sync_device.h"
#include "iree/hal/local/loaders/embedded_elf_loader.h"
#include "iree/modules/hal/module.h"
#include "iree/vm/bytecode_module.h"
#else // __UNIT_TEST__
#include "partial_iree_wrapper.h"
#endif // __UNIT_TEST__

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

/**
 * IREE wrapper custom error codes
 */
#define IREE_WRAPPER_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(IREE_WRAPPER);

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
    enum iree_hal_element_types_t hal_element_type;
    uint8_t entry_func[MAX_LENGTH_ENTRY_FUNC_NAME];
    uint8_t model_name[MAX_LENGTH_MODEL_NAME];
} MlModel;

#define BREAK_ON_IREE_ERROR(status) \
    if (!iree_status_is_ok(status)) \
    {                               \
        break;                      \
    }

#define CHECK_IREE_STATUS(status)                    \
    if (!iree_status_is_ok(status))                  \
    {                                                \
        return GENERATE_ERROR(IREE_WRAPPER, status); \
    }

/**
 * Creates context that hold modules' state
 *
 * @param model_data compiled model data
 * @param model_data_size size of compiled model data
 *
 * @returns error status
 */
status_t create_context(const uint8_t *model_data, const size_t model_data_size);

/**
 * Prepares model input buffer
 *
 * @param model_struct struct that contains model params
 * @param model_input model input
 *
 * @returns error status
 */
status_t prepare_input_buffer(const MlModel *model_struct, const uint8_t *model_input);

/**
 * Prepares model output buffer
 *
 * @returns error status
 */
status_t prepare_output_buffer();

/**
 * Runs model inference
 *
 * @returns error status
 */
status_t run_inference();

/**
 * Returns model output
 *
 * @param model_output buffer to save model output into
 *
 * @returns error status
 */
status_t get_output(uint8_t *model_output);

/**
 * Returns model stats
 *
 * @param statistics_buffer_size size of the passed buffer
 * @param statistics_buffer buffer to save stats into
 * @param statistics_size returned stats size
 *
 * @returns error status
 */
status_t get_model_stats(const size_t statistics_buffer_size, uint8_t *statistics_buffer, size_t *statistics_size);

/**
 * Clears model input buffer
 */
void release_input_buffer();

/**
 * Clears model output buffer
 */
void release_output_buffer();

#endif // IREE_RUNTIME_UTIL_IREE_WRAPPER_H_
