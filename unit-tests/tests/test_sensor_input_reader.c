/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/input_reader.h"
#include "../iree-runtime/utils/sensor_input_reader.c"
#include "mock_model.h"
#include "mock_sensor.h"
#include "mocks/sensor_mock.h"
#include "unity.h"

#define TEST_CASE(...)

void setUp(void) {}

void tearDown(void) {}

// ========================================================
// read_input
// ========================================================

/**
 * Tests if read input properly reads data from sensor
 */
void test_ReadInputShouldReadDataFromSensorAndRunModel(void)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    sensor_read_data_into_buffer_IgnoreAndReturn(STATUS_OK);
    sensor_get_buffered_data_IgnoreAndReturn(STATUS_OK);
    load_model_input_IgnoreAndReturn(STATUS_OK);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
}

TEST_CASE(0)
TEST_CASE(sizeof(sensor_mock_data_t) - 1)
TEST_CASE(sizeof(sensor_mock_data_t) + 1)
/**
 * Tests if read input fails if sensor data has invalid size
 */
void test_ReadInputShouldFailIfSensorDataHasInvalidSize(size_t sensor_data_size)
{
    status_t status = STATUS_OK;
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(INPUT_READER_STATUS_INV_ARG, status);
}

TEST_CASE(0)
TEST_CASE(sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN - 1)
TEST_CASE(sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN + 1)
/**
 * Tests if read input fails if model input has invalid size
 */
void test_ReadInputShouldFailIfModelInputHasInvalidSize(size_t model_input_size)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(INPUT_READER_STATUS_INV_ARG, status);
}

TEST_CASE(SENSOR_STATUS_INV_SENSOR)
TEST_CASE(SENSOR_STATUS_NO_SENSOR_AVAILABLE)
/**
 * Tests if read input fails if get sensor data size fails
 */
void test_ReadInputShouldFailIfGetSensorDataFails(uint32_t sensor_error)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, sensor_error);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(sensor_error, status);
}

TEST_CASE(MODEL_STATUS_INV_STATE)
TEST_CASE(MODEL_STATUS_TIMEOUT)
/**
 * Tests if read input fails if get model input size fails
 */
void test_ReadInputShouldFailIfGetModelInputSizeFails(uint32_t model_error)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, model_error);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(model_error, status);
}

TEST_CASE(SENSOR_STATUS_INV_SENSOR)
TEST_CASE(SENSOR_STATUS_NO_SENSOR_AVAILABLE)
/**
 * Tests if read input fails if data read fails
 */
void test_ReadInputShouldFailWhenSensorDataReadFails(uint32_t sensor_error)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    sensor_read_data_into_buffer_IgnoreAndReturn(sensor_error);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(sensor_error, status);
}

TEST_CASE(SENSOR_STATUS_INV_SENSOR)
TEST_CASE(SENSOR_STATUS_NO_SENSOR_AVAILABLE)
/**
 * Tests if read input fails if get sensor data buffer fails
 */
void test_ReadInputShouldFailWhenGetSensorDataBufferFails(uint32_t sensor_error)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    sensor_read_data_into_buffer_IgnoreAndReturn(STATUS_OK);
    sensor_get_buffered_data_IgnoreAndReturn(sensor_error);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(sensor_error, status);
}

TEST_CASE(MODEL_STATUS_INV_STATE)
TEST_CASE(MODEL_STATUS_TIMEOUT)
/**
 * Tests if read input fails when model input load fails
 */
void test_ReadInputShouldFailWhenLoadModelInputFails(uint32_t model_error)
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = sizeof(sensor_mock_data_t);
    size_t model_input_size = sizeof(sensor_mock_data_t) * SENSOR_MOCK_BUFFER_LEN;

    sensor_get_data_size_ExpectAndReturn(NULL, STATUS_OK);
    sensor_get_data_size_IgnoreArg_data_size();
    sensor_get_data_size_ReturnThruPtr_data_size(&sensor_data_size);

    get_model_input_size_ExpectAndReturn(NULL, STATUS_OK);
    get_model_input_size_IgnoreArg_model_input_size();
    get_model_input_size_ReturnThruPtr_model_input_size(&model_input_size);

    sensor_read_data_into_buffer_IgnoreAndReturn(STATUS_OK);
    sensor_get_buffered_data_IgnoreAndReturn(STATUS_OK);
    load_model_input_IgnoreAndReturn(model_error);

    status = read_input();

    TEST_ASSERT_EQUAL_HEX(model_error, status);
}
