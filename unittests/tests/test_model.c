#include "../iree-runtime/utils/model.h"
#include "mock_iree_wrapper.h"
#include "unity.h"

#include <string.h>

#define TEST_CASE(...)

#define MODEL_STRUCT_INPUT_LEN 784
#define MODEL_STRUCT_INPUT_SIZE 4
#define MODEL_STRUCT_OUTPUT_LEN 10
#define MODEL_STRUCT_OUTPUT_SIZE 4

#define VALID_HAL_ELEMENT_TYPE "f32"

extern MlModel g_model_struct;
extern MODEL_STATE g_model_state;

/**
 * Returns example model struct data with passed dtype.
 *
 * @param dtype dtype of model input
 *
 * @returns prepared model struct
 */
MlModel get_model_struct_data(char dtype[]);

void setUp(void) {}

void tearDown(void) {}

// ========================================================
// load_model_struct
// ========================================================

TEST_CASE("f16", IREE_HAL_ELEMENT_TYPE_FLOAT_16)
TEST_CASE("f32", IREE_HAL_ELEMENT_TYPE_FLOAT_32)
TEST_CASE("f64", IREE_HAL_ELEMENT_TYPE_FLOAT_64)
TEST_CASE("i8", IREE_HAL_ELEMENT_TYPE_INT_8)
TEST_CASE("i32", IREE_HAL_ELEMENT_TYPE_INT_32)
TEST_CASE("u32", IREE_HAL_ELEMENT_TYPE_UINT_32)
/**
 * Tests model struct parsing for valid HAL element types
 */
void test_ModelLoadModelStructShouldParseStructAndChangeModelStateForValidHALElementType(
    char *dtype, iree_hal_element_type_t hal_element_type)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(dtype);

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(hal_element_type, g_model_struct.hal_element_type);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

TEST_CASE("")
TEST_CASE("x")
TEST_CASE("f15")
TEST_CASE("i31")
TEST_CASE("f32x")
TEST_CASE("abcdefg")
/**
 * Tests model struct parsing for invalid HAL element types
 */
void test_ModelLoadModelStructShouldFailForInvalidHALElementType(char *dtype)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(dtype);
    g_model_state = MODEL_STATE_UNINITIALIZED;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

TEST_CASE(1)
TEST_CASE(MAX_MODEL_INPUT_NUM)
/**
 * Tests model struct parsing for valid input_num values
 */
void test_ModelLoadModelStructShouldParseStructAndChangeModelStateForValidInputNum(uint32_t num_input)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

/**
 * Tests model struct parsing for invalid input_num values
 */
TEST_CASE(0)
TEST_CASE(MAX_MODEL_INPUT_NUM + 1)
TEST_CASE(MAX_MODEL_INPUT_NUM + 100)
TEST_CASE(-1)
void test_ModelLoadModelStructShouldFailForInvalidInputNum(uint32_t num_input)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;
    model_struct.num_input = num_input;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

/**
 * Tests model struct parsing for valid num_output values
 */
TEST_CASE(1)
TEST_CASE(MAX_MODEL_OUTPUTS)
void test_ModelLoadModelStructShouldParseStructAndChangeModelStateForValidOutputNum(uint32_t num_output)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;
    model_struct.num_output = num_output;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

/**
 * Tests model struct parsing for invalid num_output values
 */
TEST_CASE(MAX_MODEL_OUTPUTS + 1)
TEST_CASE(MAX_MODEL_OUTPUTS + 100)
TEST_CASE(-1)
void test_ModelLoadModelStructShouldFailForInvalidOutputNum(uint32_t num_output)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;
    model_struct.num_output = num_output;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

/**
 * Tests model struct parsing for valid input_dim values
 */
TEST_CASE(0)
TEST_CASE(MAX_MODEL_INPUT_DIM)
void test_ModelLoadModelStructShouldParseStructAndChangeModelStateForValidInputDim(uint32_t input_dim)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;
    model_struct.num_input_dim[0] = input_dim;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

/**
 * Tests model struct parsing for invalid input_dim values
 */
TEST_CASE(MAX_MODEL_INPUT_DIM + 1)
TEST_CASE(MAX_MODEL_INPUT_DIM + 100)
TEST_CASE(-1)
void test_ModelLoadModelStructShouldFailForInvalidInputDim(uint32_t input_dim)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;
    model_struct.num_input_dim[0] = input_dim;

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

/**
 * Tests model struct parsing for data with invalid size
 */
TEST_CASE(sizeof(MlModel) - 1)
TEST_CASE(sizeof(MlModel) + 1)
TEST_CASE(0)
TEST_CASE(-1)
void test_ModeLoadModelStructShouldFailIfModelStructDataHasInvalidSize(size_t struct_size)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data(VALID_HAL_ELEMENT_TYPE);
    g_model_state = MODEL_STATE_UNINITIALIZED;

    model_status = load_model_struct((uint8_t *)&model_struct, struct_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

/**
 * Tests model struct parsing for invalid pointer
 */
void test_ModelLoadModelStructShouldFailForInvalidPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = MODEL_STATE_UNINITIALIZED;

    model_status = load_model_struct(NULL, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

// ========================================================
// load_model_weights
// ========================================================

TEST_CASE(1) // MODEL_STATE_STRUCT_LOADED
TEST_CASE(2) // MODEL_STATE_WEIGHTS_LOADED
TEST_CASE(3) // MODEL_STATE_INPUT_LOADED
TEST_CASE(4) // MODEL_STATE_INFERENCE_DONE
/**
 * Tests model weights loading in valid model states
 */
void test_ModelLoadModelWeightsShouldCreateContextAndChangeModelState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_weights[128];

    g_model_state = model_state;
    create_context_ExpectAndReturn(model_weights, sizeof(model_weights), IREE_WRAPPER_STATUS_OK);
    release_output_buffer_Ignore();
    release_input_buffer_Ignore();

    model_status = load_model_weights(model_weights, sizeof(model_weights));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model weights loading for invalid pointer
 */
void test_ModelLoadModelWeightsShouldFailForInvalidPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    model_status = load_model_weights(NULL, 0);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

/**
 * Tests model weights loading when model is in invalid state
 */
void test_ModelLoadWeightsShouldFailInModelStateIsUninitialized(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_weights[128];

    g_model_state = MODEL_STATE_UNINITIALIZED;

    model_status = load_model_weights(model_weights, sizeof(model_weights));

    TEST_ASSERT_EQUAL_UINT(model_status, MODEL_STATUS_INVALID_STATE);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_UNINITIALIZED, g_model_state);
}

/**
 * Tests model weights loading when IREE context creation fails
 */
void test_ModelLoadModelWeightsShouldFailIfCreateContextFails(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_weights[128];

    g_model_state = MODEL_STATE_STRUCT_LOADED;
    create_context_ExpectAndReturn(model_weights, sizeof(model_weights), IREE_WRAPPER_STATUS_ERROR);
    release_output_buffer_Ignore();
    release_input_buffer_Ignore();

    model_status = load_model_weights(model_weights, sizeof(model_weights));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_IREE_ERROR, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

// ========================================================
// load_model_input
// ========================================================

TEST_CASE(2) // MODEL_STATE_WEIGHTS_LOADED
TEST_CASE(3) // MODEL_STATE_INPUT_LOADED
TEST_CASE(4) // MODEL_STATE_INFERENCE_DONE
/**
 * Tests model input loading for valid model states
 */
void test_ModelLoadModelInputShouldPrepareInputBufferAndChangeModelState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_input[MODEL_STRUCT_INPUT_LEN * MODEL_STRUCT_INPUT_SIZE];

    g_model_state = model_state;
    prepare_input_buffer_ExpectAndReturn(&g_model_struct, model_input, IREE_WRAPPER_STATUS_OK);
    release_input_buffer_Ignore();

    model_status = load_model_input(model_input, sizeof(model_input));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model input loading when IREE buffer allocation fails
 */
void test_ModelLoadModelInputShouldFailIfPrepareInputBufferReturnsError(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_input[MODEL_STRUCT_INPUT_LEN * MODEL_STRUCT_INPUT_SIZE];

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;
    prepare_input_buffer_ExpectAndReturn(&g_model_struct, model_input, IREE_WRAPPER_STATUS_ERROR);
    release_input_buffer_Ignore();

    model_status = load_model_input(model_input, sizeof(model_input));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_IREE_ERROR, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model input loading for invalid pointer
 */
void test_ModelLoadModelInputShouldFailForInvalidInputPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    model_status = load_model_input(NULL, 0);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

TEST_CASE(0) // MODEL_STATE_UNINITIALIZED
TEST_CASE(1) // MODEL_STATE_STRUCT_LOADED
/**
 * Tests model input loading when model is in invalid state
 */
void test_ModelLoadModelInputShouldFailIfModelIsInInvalidState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_input[MODEL_STRUCT_INPUT_LEN * MODEL_STRUCT_INPUT_SIZE];

    g_model_state = model_state;

    model_status = load_model_input(model_input, sizeof(model_input));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_STATE, model_status);
    TEST_ASSERT_EQUAL_UINT(model_state, g_model_state);
}

/**
 * Tests model input loading when input size is invalid
 */
void test_ModelLoadModelInputShouldFailForInvalidInputSize(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_input[MODEL_STRUCT_INPUT_LEN * MODEL_STRUCT_INPUT_SIZE];

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    model_status = load_model_input(model_input, sizeof(model_input) / 2);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

// ========================================================
// run_model
// ========================================================

TEST_CASE(3) // MODEL_STATE_INPUT_LOADED
TEST_CASE(4) // MODEL_STATE_INFERENCE_DONE
/**
 * Tests model execution for valid model states
 */
void test_ModelRunModelShouldPrepareOutputAndRunInference(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = model_state;
    release_output_buffer_Ignore();
    prepare_output_buffer_IgnoreAndReturn(IREE_WRAPPER_STATUS_OK);
    run_inference_IgnoreAndReturn(IREE_WRAPPER_STATUS_OK);

    model_status = run_model();

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

/**
 * Tests model execution when output buffer allocation fails
 */
void test_ModelRunModelShouldFailIfPrepareOutputFails(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = MODEL_STATE_INPUT_LOADED;
    release_output_buffer_Ignore();
    prepare_output_buffer_IgnoreAndReturn(IREE_WRAPPER_STATUS_ERROR);

    model_status = run_model();

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_IREE_ERROR, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model execution when inference fails
 */
void test_ModelRunModelShouldFailIfRunInferenceFails(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = MODEL_STATE_INPUT_LOADED;
    release_output_buffer_Ignore();
    prepare_output_buffer_IgnoreAndReturn(IREE_WRAPPER_STATUS_OK);
    run_inference_IgnoreAndReturn(IREE_WRAPPER_STATUS_ERROR);

    model_status = run_model();

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_IREE_ERROR, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INPUT_LOADED, g_model_state);
}

TEST_CASE(0) // MODEL_STATE_UNINITIALIZED
TEST_CASE(1) // MODEL_STATE_STRUCT_LOADED
TEST_CASE(2) // MODEL_STATE_WEIGHTS_LOADED
/**
 * Tests model execution when model is in invalid state
 */
void test_ModelRunModelShouldFailIfModelIsInInvalidState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    g_model_state = model_state;

    model_status = run_model();

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_STATE, model_status);
    TEST_ASSERT_EQUAL_UINT(model_state, g_model_state);
}

// ========================================================
// get_model_output
// ========================================================

/**
 * Tests model get output for valid model states
 */
void test_ModelGetModelOutputShouldReturnModelOutput(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_output[MODEL_STRUCT_OUTPUT_LEN * MODEL_STRUCT_OUTPUT_SIZE];
    size_t model_output_size = 0;

    g_model_state = MODEL_STATE_INFERENCE_DONE;
    get_output_ExpectAndReturn(model_output, IREE_WRAPPER_STATUS_OK);

    model_status = get_model_output(sizeof(model_output), model_output, &model_output_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INFERENCE_DONE, g_model_state);
    TEST_ASSERT_EQUAL_UINT(MODEL_STRUCT_OUTPUT_LEN * 4, model_output_size);
}

/**
 * Tests model get output for invalid buffer pointer
 */
void test_ModelGetModelOutputShouldFailForInvalidModelOutputPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    size_t model_output_size = 0;

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    model_status = get_model_output(0, NULL, &model_output_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

/**
 * Tests model get output for invalid buffer size
 */
void test_ModelGetModelOutputShouldFailForInvalidModelOutputSizePointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_output[MODEL_STRUCT_OUTPUT_LEN * MODEL_STRUCT_OUTPUT_SIZE];

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    model_status = get_model_output(sizeof(model_output), model_output, NULL);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

TEST_CASE(0) // MODEL_STATE_UNINITIALIZED
TEST_CASE(1) // MODEL_STATE_STRUCT_LOADED
TEST_CASE(2) // MODEL_STATE_WEIGHTS_LOADED
TEST_CASE(3) // MODEL_STATE_INPUT_LOADED
/**
 * Tests model get output when model is in invalid state
 */
void test_ModelGetModelOutputShouldFailIfModelIsInInvalidState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_output[MODEL_STRUCT_OUTPUT_LEN * MODEL_STRUCT_OUTPUT_SIZE];
    size_t model_output_size = 0;

    g_model_state = model_state;

    model_status = get_model_output(sizeof(model_output), model_output, &model_output_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_STATE, model_status);
    TEST_ASSERT_EQUAL_UINT(model_state, g_model_state);
}

// ========================================================
// get_statistics
// ========================================================

TEST_CASE(2) // MODEL_STATE_WEIGHTS_LOADED
TEST_CASE(3) // MODEL_STATE_INPUT_LOADED
TEST_CASE(4) // MODEL_STATE_INFERENCE_DONE
/**
 * Tests model get statistics for valid model states
 */
void test_ModelGetStatisticsShouldReturnModelStatistics(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t statistics_buffer[128];
    size_t statistics_size = 0;

    g_model_state = model_state;
    get_model_stats_ExpectAndReturn(sizeof(statistics_buffer), statistics_buffer, &statistics_size,
                                    IREE_WRAPPER_STATUS_OK);

    model_status = get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(model_state, g_model_state);
}

/**
 * Tests model get statistics for invalid buffer pointer
 */
void test_ModelGetStatisticsShouldFailForInvalidStatisticsBufferPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    size_t statistics_size = 0;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    model_status = get_statistics(0, NULL, &statistics_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model get statistics for invalid size pointer
 */
void test_ModelGetStatisticsShouldFailForInvalidStatisticsSizePointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t statistics_buffer[128];

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    model_status = get_statistics(sizeof(statistics_buffer), statistics_buffer, NULL);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

TEST_CASE(0) // MODEL_STATE_UNINITIALIZED
TEST_CASE(1) // MODEL_STATE_STRUCT_LOADED
/**
 * Tests model get statistics when model is in invalid state
 */
void test_ModelGetStatisticsShouldFailIfModelInInvalidState(uint32_t model_state)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t statistics_buffer[128];
    size_t statistics_size = 0;

    g_model_state = model_state;

    model_status = get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_STATE, model_status);
    TEST_ASSERT_EQUAL_UINT(model_state, g_model_state);
}

/**
 * Tests model get statistics when get model stats fails
 */
void test_ModelGetStatisticsShouldFailIfGetModelStatsFails(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t statistics_buffer[128];
    size_t statistics_size = 0;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;
    get_model_stats_ExpectAndReturn(sizeof(statistics_buffer), statistics_buffer, &statistics_size,
                                    IREE_WRAPPER_STATUS_ERROR);

    model_status = get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_IREE_ERROR, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

// ========================================================
// helper functions
// ========================================================

MlModel get_model_struct_data(char dtype[])
{
    MlModel model_struct = {
        1,
        {4},
        {1, 28, 28, 1},
        {MODEL_STRUCT_INPUT_LEN},
        {MODEL_STRUCT_INPUT_SIZE},
        1,
        {MODEL_STRUCT_OUTPUT_LEN},
        MODEL_STRUCT_OUTPUT_SIZE,
        0,
        "module.main",
        "module",
    };
    strncpy(&model_struct.hal_element_type, dtype, 4);

    return model_struct;
}
