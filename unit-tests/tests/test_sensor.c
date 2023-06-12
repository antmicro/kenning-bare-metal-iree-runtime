/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/sensor.h"
#include "mock_adxl345.h"
#include "mock_i2c.h"
#include "mock_sensor_mock.h"
#include "mock_utils.h"
#include "unity.h"

#define TEST_CASE(...)

uint32_t g_mock_csr = 0;
extern sensor_data_t g_sensor_data_buffer[];
extern ut_static size_t g_sensor_data_buffer_idx;

/**
 * Callback that is called every read from timer register. It simulates time passing by incrementing this register.
 */
void mock_csr_read_callback();

void setUp(void) { g_sensor_data_buffer_idx = 0; }

void tearDown(void) {}

// ========================================================
// sensor_init
// ========================================================

void test_SensorInitShouldPassForValidSensor(void)
{
    status_t status = STATUS_OK;
    uint8_t device_id = SENSOR_MOCK_DEVICE_ID;

    i2c_read_target_register_ExpectAndReturn(SENSOR_I2C_ADDRESS, 0x0, NULL, STATUS_OK);
    i2c_read_target_register_IgnoreArg_data();
    i2c_read_target_register_ReturnThruPtr_data(&device_id);

    status = sensor_init();

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
}

void test_SensorInitShouldFailForInvalidSensor(void)
{
    status_t status = STATUS_OK;
    uint8_t invalid_device_id = SENSOR_MOCK_DEVICE_ID - 1;

    i2c_read_target_register_ExpectAndReturn(SENSOR_I2C_ADDRESS, 0x0, NULL, STATUS_OK);
    i2c_read_target_register_IgnoreArg_data();
    i2c_read_target_register_ReturnThruPtr_data(&invalid_device_id);

    status = sensor_init();

    TEST_ASSERT_EQUAL_HEX(SENSOR_STATUS_INV_SENSOR, status);
}

TEST_CASE(I2C_STATUS_ERROR)
TEST_CASE(I2C_STATUS_INV_ARG)
void test_SensorInitShouldFailIfRegisterReadFails(status_t i2c_error)
{
    status_t status = STATUS_OK;

    i2c_read_target_register_IgnoreAndReturn(i2c_error);

    status = sensor_init();

    TEST_ASSERT_EQUAL_HEX(i2c_error, status);
}

// ========================================================
// sensor_get_device_id
// ========================================================

/**
 * Tests if get device ID reads proper register
 */
void test_GetDeviceIDShouldReadProperRegister(void)
{
    status_t status = STATUS_OK;
    uint8_t device_id = 0;

    i2c_read_target_register_ExpectAndReturn(SENSOR_I2C_ADDRESS, 0x0, &device_id, STATUS_OK);

    status = sensor_get_device_id(&device_id);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
}

/**
 * Tests if get device ID fails for invalid pointer
 */
void test_GetDeviceIDShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = sensor_get_device_id(NULL);

    TEST_ASSERT_EQUAL_HEX(SENSOR_STATUS_INV_PTR, status);
}

TEST_CASE(I2C_STATUS_ERROR)
TEST_CASE(I2C_STATUS_INV_ARG)
/**
 * Tests if get device ID fails when register read fails
 */
void test_GetDeviceIDShouldFailWhenRegisterReadFails(status_t i2c_status)
{
    status_t status = STATUS_OK;
    uint8_t device_id = 0;

    i2c_read_target_register_IgnoreAndReturn(i2c_status);

    status = sensor_get_device_id(&device_id);

    TEST_ASSERT_EQUAL_HEX(i2c_status, status);
}

// ========================================================
// sensor_get_data_size
// ========================================================

/**
 * Tests if get data size returns size retrieved from sensors table
 */
void test_GetDataSizeShouldReturnSizeRetrievedFromSensorsTable(void)
{
    status_t status = STATUS_OK;
    size_t data_size = 0;

    status = sensor_get_data_size(&data_size);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT(sizeof(sensor_mock_data_t), data_size);
}

/**
 * Tests if get data size fails for invalid pointer
 */
void test_GetDataSizeShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    status = sensor_get_data_size(NULL);

    TEST_ASSERT_EQUAL_HEX(SENSOR_STATUS_INV_PTR, status);
}

// ========================================================
// sensor_read_data_into_buffer
// ========================================================

TEST_CASE(0, 1.0f, 2.0f)
TEST_CASE(1, 123.0f, 321.0f)
TEST_CASE(10, -5.0f, -10.0f)
/**
 * Tests if read data into buffer properly inserts read data from sensor into buffer
 */
void test_ReadDataIntoBufferShouldReadDataFromSensorAndInsertItIntoBuffer(size_t buffer_idx, float a, float b)
{
    status_t status = STATUS_OK;
    sensor_mock_data_t data = {.a = a, .b = b};

    g_sensor_data_buffer_idx = buffer_idx;

    sensor_mock_read_data_ExpectAndReturn(NULL, STATUS_OK);
    sensor_mock_read_data_IgnoreArg_data();
    sensor_mock_read_data_ReturnThruPtr_data(&data);

    status = sensor_read_data_into_buffer();

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT(buffer_idx + 1, g_sensor_data_buffer_idx);
    TEST_ASSERT_EQUAL(a, g_sensor_data_buffer[buffer_idx].a);
    TEST_ASSERT_EQUAL(b, g_sensor_data_buffer[buffer_idx].b);
}

/**
 * Tests if read data into buffer sets index to zero when it reaches the end of the buffer
 */
void test_ReadDataIntoBufferShouldSetIndexToZeroWhenReachesBufferEnd(void)
{
    status_t status = STATUS_OK;

    g_sensor_data_buffer_idx = SENSOR_BUFFER_LEN - 1;

    sensor_mock_read_data_IgnoreAndReturn(STATUS_OK);

    status = sensor_read_data_into_buffer();

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT(0, g_sensor_data_buffer_idx);
}

TEST_CASE(SENSOR_MOCK_STATUS_INV_ARG)
TEST_CASE(SENSOR_MOCK_STATUS_ERROR)
/**
 * Tests if read data into buffer fails when sensor read fails
 */
void test_ReadDataIntoBufferShouldFailWhenSensorReadFails(status_t sensor_error)
{
    status_t status = STATUS_OK;

    sensor_mock_read_data_IgnoreAndReturn(sensor_error);

    status = sensor_read_data_into_buffer();

    TEST_ASSERT_EQUAL_HEX(sensor_error, status);
}

// ========================================================
// sensor_get_buffered_data
// ========================================================

TEST_CASE(0, 1.0f, 2.0f)
TEST_CASE(1, 123.0f, 321.0f)
TEST_CASE(10, -5.0f, -10.0f)
/**
 * Tests if get buffered data properly writes data to provided buffer
 */
void test_GetBufferedDataShouldCopySensorDataToProvidedBuffer(size_t buffer_idx, float a, float b)
{
    status_t status = STATUS_OK;
    sensor_mock_data_t buffer[SENSOR_MOCK_BUFFER_LEN];

    g_sensor_data_buffer[buffer_idx].a = a;
    g_sensor_data_buffer[buffer_idx].b = b;
    g_sensor_data_buffer_idx = buffer_idx;

    status = sensor_get_buffered_data(SENSOR_MOCK_BUFFER_LEN * sizeof(sensor_mock_data_t), (uint8_t *)buffer);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL(a, buffer[0].a);
    TEST_ASSERT_EQUAL(b, buffer[0].b);
}

TEST_CASE(0)
TEST_CASE(4)
TEST_CASE(-1)
/**
 * Tests if get buffered data fails when provided buffer has invalid size
 */
void test_GetBufferedDataShouldFailForInvalidBufferSize(size_t buffer_size)
{
    status_t status = STATUS_OK;
    sensor_mock_data_t buffer[SENSOR_MOCK_BUFFER_LEN];

    status = sensor_get_buffered_data(buffer_size, (uint8_t *)buffer);

    TEST_ASSERT_EQUAL_HEX(SENSOR_STATUS_INV_ARG, status);
}

/**
 * Tests if get buffered data fails when provided buffer is invalid
 */
void test_GetBufferedDataShouldFailForInvalidBufferPointer()
{
    status_t status = STATUS_OK;
    sensor_mock_data_t buffer[SENSOR_MOCK_BUFFER_LEN];

    status = sensor_get_buffered_data(SENSOR_MOCK_BUFFER_LEN * sizeof(sensor_mock_data_t), NULL);

    TEST_ASSERT_EQUAL_HEX(SENSOR_STATUS_INV_PTR, status);
}

// ========================================================
// mocks
// ========================================================

void mock_csr_read_callback() { g_mock_csr += TIMER_CLOCK_FREQ >> 4; }
