/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/i2c.h"
#include "unity.h"

#define TEST_CASE(...)

uint8_t g_write_byte_buffer_byte[8];
uint32_t g_write_byte_buffer_format[8];
size_t g_write_byte_buffer_len = 0;
i2c_registers_t g_mock_i2c_registers;
extern i2c_t g_i2c;

void setUp(void)
{
    g_i2c.initialized = false;
    memset(&g_mock_i2c_registers, 0, sizeof(g_mock_i2c_registers));
}

void tearDown(void) {}

// ========================================================
// i2c_init
// ========================================================

TEST_CASE(0 /* I2C_SPEED_STANDARD */, 41)
TEST_CASE(1 /* I2C_SPEED_FAST */, 41)
TEST_CASE(2 /* I2C_SPEED_FAST_PLUS */, 41)
/**
 * Tests I2C initialization for valid parameters
 */
void test_I2CInitShouldSucceedForValidConfig(uint32_t speed, uint32_t clock_period_nanos)
{
    status_t status = STATUS_OK;
    i2c_config_t config = {.speed = speed, .clock_period_nanos = clock_period_nanos};

    status = i2c_init(&config);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_TRUE(g_i2c.initialized);
    TEST_ASSERT_TRUE(GET_REG_FIELD(g_mock_i2c_registers.FIFO_CTRL, FIFO_CTRL_RXRST));
    TEST_ASSERT_TRUE(GET_REG_FIELD(g_mock_i2c_registers.FIFO_CTRL, FIFO_CTRL_FMTRST));
    TEST_ASSERT_TRUE(GET_REG_FIELD(g_mock_i2c_registers.FIFO_CTRL, FIFO_CTRL_ACQRST));
    TEST_ASSERT_TRUE(GET_REG_FIELD(g_mock_i2c_registers.FIFO_CTRL, FIFO_CTRL_TXRST));
    TEST_ASSERT_TRUE(GET_REG_FIELD(g_mock_i2c_registers.CTRL, CTRL_ENABLEHOST));
}

/**
 * Tests I2C initialization for invalid config pointer
 */
void test_I2CInitShouldFailForInvalidConfigPointer(void)
{
    status_t status = STATUS_OK;

    status = i2c_init(NULL);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_PTR, status);
    TEST_ASSERT_FALSE(g_i2c.initialized);
}

TEST_CASE(-1, 41)
TEST_CASE(3, 41)
/**
 * Tests I2C initialization for invalid speed
 */
void test_I2CInitShouldFailForInvalidSpeedInConfig(uint32_t speed, uint32_t clock_period_nanos)
{
    status_t status = STATUS_OK;
    i2c_config_t config = {.speed = speed, .clock_period_nanos = clock_period_nanos};

    status = i2c_init(&config);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_ARG, status);
    TEST_ASSERT_FALSE(g_i2c.initialized);
}

// ========================================================
// i2c_get_status
// ========================================================

TEST_CASE(0x0)
TEST_CASE(0xAAA)
TEST_CASE(0xCCC)
TEST_CASE(0x3E0)
TEST_CASE(0x3FF)
/**
 * Tests if get status reads flags from I2C status register
 */
void test_I2CGetStatusShouldReadDataFromStatusRegister(uint32_t status_reg)
{
    status_t status = STATUS_OK;
    i2c_status_t i2c_status;

    g_i2c.initialized = true;
    g_mock_i2c_registers.STATUS = status_reg;

    status = i2c_get_status(&i2c_status);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_FMT_FULL), i2c_status.fmt_full);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_RX_FULL), i2c_status.rx_full);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_FMT_EMPTY), i2c_status.fmt_empty);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_HOST_IDLE), i2c_status.host_idle);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_TARGET_IDLE), i2c_status.target_idle);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_RX_EMPTY), i2c_status.rx_empty);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_TX_FULL), i2c_status.tx_full);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_ACQ_FULL), i2c_status.acq_full);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_TX_EMPTY), i2c_status.tx_empty);
    TEST_ASSERT_EQUAL(GET_REG_FIELD(status_reg, STATUS_ACQ_EMPTY), i2c_status.acq_empty);
}

/**
 * Tests if get status fails for invalid pointer
 */
void test_I2CGetStatusShouldFailForInvalidStatusPointer(void)
{
    status_t status = STATUS_OK;

    g_i2c.initialized = true;

    status = i2c_get_status(NULL);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_PTR, status);
}

/**
 * Tests if get status fails when I2C is not initialized
 */
void test_I2CGetStatusShouldFailWhenI2CIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    i2c_status_t i2c_status;

    status = i2c_get_status(&i2c_status);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

// ========================================================
// i2c_write_byte
// ========================================================

TEST_CASE(0 /* I2C_FORMAT_START */)
TEST_CASE(1 /* I2C_FORMAT_TX */)
TEST_CASE(2 /* I2C_FORMAT_TX_STOP */)
TEST_CASE(3 /* I2C_FORMAT_RX */)
TEST_CASE(4 /* I2C_FORMAT_RX_CONT */)
TEST_CASE(5 /* I2C_FORMAT_RX_STOP */)
/**
 * Tests if byte is written to fdata register with proper flags
 */
void test_I2CWriteByteShouldWriteDataToFDATAWithProperFlags(uint32_t format)
{
    status_t status = STATUS_OK;
    uint8_t byte = 'x';
    i2c_format_flags_t flags = {.no_ack_ok = true};

    switch (format)
    {
    case I2C_FORMAT_START:
        flags.start = true;
        break;
    case I2C_FORMAT_TX:
        break;
    case I2C_FORMAT_TX_STOP:
        flags.stop = true;
        break;
    case I2C_FORMAT_RX:
        flags.read = true;
        break;
    case I2C_FORMAT_RX_CONT:
        flags.read = true;
        flags.read_cont = true;
        break;
    case I2C_FORMAT_RX_STOP:
        flags.read = true;
        flags.stop = true;
        break;
    }

    g_i2c.initialized = true;

    status = i2c_write_byte(byte, format);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(byte, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_FBYTE));
    TEST_ASSERT_EQUAL(flags.start, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_START));
    TEST_ASSERT_EQUAL(flags.stop, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_STOP));
    TEST_ASSERT_EQUAL(flags.read, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_READ));
    TEST_ASSERT_EQUAL(flags.read_cont, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_RCONT));
    TEST_ASSERT_EQUAL(flags.no_ack_ok, GET_REG_FIELD(g_mock_i2c_registers.FDATA, FDATA_NAKOK));
}

TEST_CASE(-1)
TEST_CASE(6)
/**
 * Tests if write byte fails for invalid format
 */
void test_I2CWriteByteShouldFailForInvalidFormat(uint32_t format)
{
    status_t status = STATUS_OK;
    uint8_t byte = 'x';

    g_i2c.initialized = true;

    status = i2c_write_byte(byte, format);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_ARG, status);
}

/**
 * Tests if write byte fails when I2C is not initialized
 */
void test_I2CWriteByteShouldFailWhenI2CIsNotInitialized()
{
    status_t status = STATUS_OK;
    uint8_t byte = 'x';

    status = i2c_write_byte(byte, 0);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

// ========================================================
// i2c_read_byte
// ========================================================

TEST_CASE('a')
TEST_CASE('b')
TEST_CASE(0x0)
TEST_CASE(0xFF)
/**
 * Tests if read byte reads data from proper register
 */
void test_I2CReadByteShouldReadDataFromRDATARegister(uint8_t rdata)
{
    status_t status = STATUS_OK;
    uint8_t byte = 0;

    g_i2c.initialized = true;
    g_mock_i2c_registers.RDATA = rdata;

    status = i2c_read_byte(&byte);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(rdata, byte);
}

/**
 * Tests if read byte fails if byte pointer is invalid
 */
void test_I2CReadByteShouldFailForInvalidBytePointer()
{
    status_t status = STATUS_OK;

    g_i2c.initialized = true;

    status = i2c_read_byte(NULL);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_PTR, status);
}

/**
 * Tests if read byte fails when I2C is not initialized
 */
void test_I2CReadByteShouldFailWhenI2CIsNotInitialized()
{
    status_t status = STATUS_OK;
    uint8_t byte = 0;

    status = i2c_read_byte(&byte);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

// ========================================================
// i2c_write_target_register
// ========================================================

/**
 * Tests if write target register succeeds when I2C is initialized
 */
void test_I2CWriteTargetRegisterShouldSucceedWhenI2CIsInitialized(void)
{
    status_t status = STATUS_OK;

    g_i2c.initialized = true;

    status = i2c_write_target_register(0x1D, 0x31, 0xAB);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
}

/**
 * Tests if write target register fails when I2C is not initialized
 */
void test_I2CWriteTargetRegisterShouldFailWhenI2CIsNotInitialized(void)
{
    status_t status = STATUS_OK;

    status = i2c_write_target_register(0x1D, 0x31, 0xAB);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

// ========================================================
// i2c_read_target_register
// ========================================================

TEST_CASE(0x0)
TEST_CASE('a')
TEST_CASE('a')
TEST_CASE(0xFF)
/**
 * Tests if read target register succeeds when I2C is initialized
 */
void test_I2CReadTargetRegisterShouldSucceedWhenI2CIsInitialized(uint8_t rdata)
{
    status_t status = STATUS_OK;
    uint8_t byte = 0;

    g_i2c.initialized = true;
    g_i2c.registers->RDATA = rdata;

    status = i2c_read_target_register(0x1D, 0x31, &byte);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(rdata, byte);
}

/**
 * Tests if read target register fails when I2C is not initialized
 */
void test_I2CReadTargetRegisterShouldFailWhenI2CIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    uint8_t byte = 0;

    status = i2c_read_target_register(0x1D, 0x31, &byte);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

/**
 * Tests if read target register fails for invalid pointer
 */
void test_I2CReadTargetRegisterShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    g_i2c.initialized = true;

    status = i2c_read_target_register(0x1D, 0x31, NULL);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_PTR, status);
}

// ========================================================
// i2c_read_target_registers
// ========================================================

TEST_CASE(2, 0x0)
TEST_CASE(4, 'a')
TEST_CASE(8, 'a')
TEST_CASE(16, 0xFF)
/**
 * Tests if read target registers succeeds when I2C is initialized
 */
void test_I2CReadTargetRegistersShouldSucceedWhenI2CIsInitialized(size_t count, uint8_t rdata)
{
    status_t status = STATUS_OK;
    uint8_t data[16];

    g_i2c.initialized = true;
    g_i2c.registers->RDATA = rdata;

    status = i2c_read_target_registers(0x1D, 0x31, count, data);

    TEST_ASSERT_EQUAL_HEX(STATUS_OK, status);
    TEST_ASSERT_EACH_EQUAL_UINT8(rdata, data, count);
}

/**
 * Tests if read target registers fails when I2C is not initialized
 */
void test_I2CReadTargetRegistersShouldFailWhenI2CIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    uint8_t data[4];

    status = i2c_read_target_registers(0x1D, 0x31, 4, data);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_UNINIT, status);
}

/**
 * Tests if read target registers fails for invalid pointer
 */
void test_I2CReadTargetRegistersShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    g_i2c.initialized = true;

    status = i2c_read_target_registers(0x1D, 0x31, 4, NULL);

    TEST_ASSERT_EQUAL_HEX(I2C_STATUS_INV_PTR, status);
}
