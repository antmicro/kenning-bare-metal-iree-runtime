/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/adxl345.h"
#include "mock_i2c.h"
#include "unity.h"

#define TEST_CASE(...)

#define ADXL345_ADDRESS (0x1D)
#define DATA_SIZE (4)

status_t mock_i2c_read_target_register(uint8_t target_id, uint8_t address, uint8_t *data, int num_calls);

// ========================================================
// adxl345_read_data
// ========================================================

/**
 * Tests if read data reads data from proper address
 */
void test_ReadDataShouldReadRegistersStartingFromX0(void)
{
    status_t status = STATUS_OK;
    adxl345_data_t data;

    i2c_read_target_registers_ExpectAndReturn(ADXL345_ADDRESS, ADXL345_DATA_X0, NULL, NULL, STATUS_OK);
    i2c_read_target_registers_IgnoreArg_count();
    i2c_read_target_registers_IgnoreArg_data();

    status = adxl345_read_data(&data);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
}

/**
 * Tests if read data fails if data pointer is invalid
 */
void test_ReadDataShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;
    adxl345_data_t data;

    status = adxl345_read_data(NULL);

    TEST_ASSERT_EQUAL_HEX(ADXL345_STATUS_INV_PTR, status);
}

TEST_CASE(I2C_STATUS_ERROR)
TEST_CASE(I2C_STATUS_TIMEOUT)
/**
 * Tests if read data fails when register read fails
 */
void test_ReadDataShouldFailWhenRegisterReadFails(status_t i2c_error)
{
    status_t status = STATUS_OK;
    adxl345_data_t data;

    i2c_read_target_registers_ExpectAndReturn(ADXL345_ADDRESS, ADXL345_DATA_X0, NULL, NULL, i2c_error);
    i2c_read_target_registers_IgnoreArg_count();
    i2c_read_target_registers_IgnoreArg_data();

    status = adxl345_read_data(&data);

    TEST_ASSERT_EQUAL_HEX(i2c_error, status);
}
