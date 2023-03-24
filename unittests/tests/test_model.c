#include "../iree-runtime/utils/model.h"
#include "iree/hal/drivers/local_sync/mock_sync_device.h"
#include "iree/hal/local/loaders/mock_vmvx_module_loader.h"
#include "iree/modules/hal/mock_module.h"
#include "iree/vm/mock_bytecode_module.h"
#include "mock_springbok.h"
#include "unity.h"

#include <string.h>

extern MlModel g_model_struct;
extern MODEL_STATE g_model_state;

MlModel get_valid_model_struct(char[] dtype);

void setUp(void) {}

void tearDown(void) {}

void test_ModelLoadModelStructShouldParseValidStruct(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct("f32");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(IREE_HAL_ELEMENT_TYPE_FLOAT_32, g_model_struct.hal_element_type);

    model_struct = get_model_struct("i8");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_OK, model_status);
    TEST_ASSERT_EQUAL_UINT(IREE_HAL_ELEMENT_TYPE_INT_8, g_model_struct.hal_element_type);
}

void test_ModelLoadModelStructShouldFailForInvalidHALElementType(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct("x");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT);
}

void test_ModeLoadModelStructShouldFailIfModelStructDataHasInvalidSize(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    MlModel model_struct = get_model_struct("f32");

    model_status = load_model_struct((uint8_t *)&model_struct, sizeof(MlModel) - 1);

    TEST_ASSERT_EQUAL_UINT(MODEL_STATUS_INVALID_ARGUMENT, model_status);
}

void test_ModelLoadModelStructShouldFailForInvalidPointer(void)
{
    MODEL_STATUS model_status = MODEL_STATUS_OK;

    model_status = load_model_struct(NULL, sizeof(MlModel));

    TEST_ASSERT_EQUAL_UINT(model_status, MODEL_STATUS_INVALID_POINTER);
}

MlModel get_valid_model_struct_data(char dtype[])
{
    MlModel model_struct = {
        1, {4}, {1, 28, 28, 1}, {784}, {4}, 1, {10}, 4, 0, "module.main", "module",
    };
    strncpy(&model_struct.hal_element_type, dtype, 4);

    return model_struct;
}
