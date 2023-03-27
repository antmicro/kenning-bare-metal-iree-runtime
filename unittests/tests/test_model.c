#include "../iree-runtime/utils/model.h"
#include "mock_iree_wrapper.h"
#include "unity.h"

#include <string.h>

extern MlModel g_model_struct;
extern MODEL_STATE g_model_state;

MlModel get_model_struct_data(char dtype[]);

void setUp(void) {}

void tearDown(void) {}

void test_ModelLoadModelStructShouldParseValidStructAndChangeModelState(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data("f32");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(IREE_HAL_ELEMENT_TYPE_FLOAT_32, g_model_struct.hal_element_type);

    model_struct = get_model_struct_data("i8");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(IREE_HAL_ELEMENT_TYPE_INT_8, g_model_struct.hal_element_type);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

void test_ModelLoadModelStructShouldFailForInvalidHALElementType(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data("x");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
}

void test_ModeLoadModelStructShouldFailIfModelStructDataHasInvalidSize(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct_data("f32");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel) - 1);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
}

void test_ModelLoadModelStructShouldFailForInvalidPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    model_status = load_model_struct(NULL, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
}

void test_ModelLoadModelWeightsShouldCreateContextAndChangeModelState(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_weights[128];

    g_model_state = MODEL_STATE_STRUCT_LOADED;
    create_context_ExpectAndReturn(model_weights, sizeof(model_weights), IREE_WRAPPER_STATUS_OK);
    release_output_buffer_Ignore();
    release_input_buffer_Ignore();

    model_status = load_model_weights(model_weights, sizeof(model_weights));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

void test_ModelLoadModelWeightsShouldFailForInvalidPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    model_status = load_model_weights(NULL, 0);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_POINTER, model_status);
}

void test_ModelLoadWeightsShouldFailInModelStateIsUninitialized(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;
    uint8_t model_weights[128];

    g_model_state = MODEL_STATE_UNINITIALIZED;

    model_status = load_model_weights(model_weights, sizeof(model_weights));

    TEST_ASSERT_EQUAL_UINT(model_status, MODEL_STATUS_INVALID_STATE);
}

MlModel get_model_struct_data(char dtype[])
{
    MlModel model_struct = {
        1, {4}, {1, 28, 28, 1}, {784}, {4}, 1, {10}, 4, 0, "module.main", "module",
    };
    strncpy(&model_struct.hal_element_type, dtype, 4);

    return model_struct;
}
