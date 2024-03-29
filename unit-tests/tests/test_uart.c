/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../iree-runtime/utils/uart.h"
#include "unity.h"

#include <string.h>

#define TEST_CASE(...)

uart_registers_t g_mock_uart_registers;
uint32_t g_mock_csr = 0;
extern uart_t g_uart;

/**
 * Callback that is called every read from timer register. It simulates time passing by incrementing this register.
 */
void mock_csr_read_callback();

/**
 * Prepares example valid UART config
 *
 * @returns valid UART config
 */
static uart_config_t get_valid_uart_config_t();

/**
 * Clears transmit FIFO full flag in UART flag register
 */
static void clear_FR_TXFF_flag();

/**
 * Clears receiver FIFO empty flag in UART flag register
 */
static void clear_FR_RXFE_flag();

/**
 * Sets receiver FIFO empty flag in UART flag register
 */
static void set_FR_RXFE_flag();

/**
 * Sets error flag in receive status / error clear register
 */
static void clear_RSRECR_ERR_flag();

/**
 * Clears error flag in receive status / error clear register
 */
static void set_RSRECR_ERR_flag();

void setUp(void)
{
    g_uart.initialized = false;
    memset(&g_mock_uart_registers, 0, sizeof(g_mock_uart_registers));
    clear_FR_RXFE_flag();
    clear_FR_TXFF_flag();
    clear_RSRECR_ERR_flag();
}

void tearDown(void) {}

// ========================================================
// uart_init
// ========================================================

TEST_CASE(8, 1, 110)
TEST_CASE(7, 2, 600)
TEST_CASE(6, 1, 9600)
TEST_CASE(5, 2, 38400)
TEST_CASE(6, 2, 115200)
/**
 * Tests UART initialization for valid parameters
 */
void test_UARTInitShouldSucceedForValidConfig(uint8_t data_bits, uint8_t stop_bits, uint32_t baudrate)
{
    status_t status = STATUS_OK;
    uart_config_t config = get_valid_uart_config_t();

    g_uart.initialized = false;
    config.data_bits = data_bits;
    config.stop_bits = stop_bits;
    config.baudrate = baudrate;

    status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_TRUE(g_uart.initialized);
}

/**
 * Tests UART initialization for invalid config pointer
 */
void test_UARTInitShouldFailForInvalidConfigPointer(void)
{
    status_t status = STATUS_OK;

    g_uart.initialized = false;

    status = uart_init(NULL);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_PTR, status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

TEST_CASE(0)
TEST_CASE(4)
TEST_CASE(9)
TEST_CASE(100)
TEST_CASE(-1)
/**
 * Tests UART initialization for invalid data bits config
 */
void test_UARTInitShouldFailForInvalidDataBitsInConfig(uint8_t data_bits)
{
    status_t status = STATUS_OK;
    uart_config_t config = get_valid_uart_config_t();

    g_uart.initialized = false;
    config.data_bits = data_bits;

    status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_ARG_WORDSIZE, status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

TEST_CASE(0)
TEST_CASE(3)
TEST_CASE(100)
TEST_CASE(10000)
TEST_CASE(-1)
/**
 * Tests UART initialization for invalid stop bits config
 */
void test_UARTInitShouldFailForInvalidStopBitsInConfig(uint8_t stop_bits)
{
    status_t status = STATUS_OK;
    uart_config_t config = get_valid_uart_config_t();

    g_uart.initialized = false;
    config.stop_bits = stop_bits;

    status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_ARG_STOP_BITS, status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

TEST_CASE(0u)
TEST_CASE(500000u)
TEST_CASE(-1)
/**
 * Tests UART initialization for invalid baudrate bits config
 */
void test_UARTInitShouldFailForInvalidBaudrateConfig(uint32_t baudrate)
{
    status_t status = STATUS_OK;
    uart_config_t config = get_valid_uart_config_t();

    g_uart.initialized = false;
    config.baudrate = baudrate;

    status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_ARG_BAUDRATE, status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

// ========================================================
// uart_putchar
// ========================================================

TEST_CASE('x')
TEST_CASE('\0')
TEST_CASE('\b')
/**
 * Tests if UART put char writes to proper register
 */
void test_UARTPutCharShouldWriteToDataRegister(uint8_t c)
{
    status_t status = STATUS_OK;

    g_uart.initialized = true;

    status = uart_putchar(c);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(c, g_mock_uart_registers.DR & DR_DATA_MASK);
}

/**
 * Tests if UART put char fails when UART is not initialized
 */
void test_UARTPutCharShouldFailAndDontWriteToDRIfUARTIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    const uint8_t c = 'c';
    const uint32_t dr = 0xABCD;

    g_uart.initialized = false;
    g_mock_uart_registers.DR = dr;

    status = uart_putchar(c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINIT, status);
    TEST_ASSERT_EQUAL_UINT32(dr, g_mock_uart_registers.DR);
}

// ========================================================
// uart_write
// ========================================================

TEST_CASE("some data")
TEST_CASE("\b")
TEST_CASE("\0")
/**
 * Tests if UART write writes to proper register
 */
void test_UARTWriteShouldWriteStringToDataRegister(char *data)
{
    status_t status = STATUS_OK;

    g_uart.initialized = true;

    status = uart_write(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(data[sizeof(data) - 1], g_mock_uart_registers.DR & DR_DATA_MASK);
}

/**
 * Tests if UART write fails if data pointer is invalid
 */
void test_UARTWriteShouldFailIfDataPointerIsInvalid(void)
{
    status_t status = STATUS_OK;

    g_uart.initialized = true;

    status = uart_write(NULL, 1);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_PTR, status);
}

/**
 * Tests if UART write does nothing for no data
 */
void test_UARTWriteShouldSucceedAndDontWriteToDRForNoData(void)
{
    status_t status = STATUS_OK;
    const uint8_t data[] = "";
    const uint32_t dr = 0xABCD;

    g_uart.initialized = true;
    g_mock_uart_registers.DR = dr;

    status = uart_write(data, 0);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT32(dr, g_mock_uart_registers.DR);
}

/**
 * Tests if UART write fails when UART is not initalized
 */
void test_UARTWriteShouldFailAndDontWriteToDRIfUARTIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    const uint8_t data[] = "some data";
    const uint32_t dr = 0xABCD;

    g_uart.initialized = false;
    g_mock_uart_registers.DR = dr;

    status = uart_write(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINIT, status);
    TEST_ASSERT_EQUAL_UINT32(dr, g_mock_uart_registers.DR);
}

// ========================================================
// uart_getchar
// ========================================================

TEST_CASE('x')
TEST_CASE('\0')
TEST_CASE('\b')
/**
 * Tests if UART get char reads from proper register
 */
void test_UARTGetCharShouldReadDataFromDR(char c)
{
    status_t status = STATUS_OK;
    const uint32_t dr = MASKED_OR_32(0xABCD, c, DR_DATA_MASK);

    g_uart.initialized = true;
    g_mock_uart_registers.DR = dr;

    status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8(dr & DR_DATA_MASK, c);
}

/**
 * Tests if UART get char fails when UART is not initalized
 */
void test_UARTGetCharShouldFailIfUARTIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    g_uart.initialized = false;
    g_mock_uart_registers.DR = dr;

    status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINIT, status);
}

/**
 * Tests if UART get char return no data when receive FIFO is empty
 */
void test_UARTGetCharShouldReturnNoDataIfRXFEFlagIsSet(void)
{
    status_t status = STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    set_FR_RXFE_flag();
    g_uart.initialized = true;
    g_mock_uart_registers.DR = dr;

    status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_NO_DATA, status);
}

/**
 * Tests if UART get char fails when UART is not initalized
 */
void test_UARTGetCharShouldFailWhenUARTIsNotInitializedAndRXFEFlagIsSet(void)
{
    status_t status = STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    set_FR_RXFE_flag();
    g_uart.initialized = false;
    g_mock_uart_registers.DR = dr;

    status = uart_getchar(&c);

    TEST_ASSERT_TRUE((status & UART_STATUS_UNINIT) | (status | UART_STATUS_NO_DATA));
}

/**
 * Tests if UART get char fails if UART error occurs
 */
void test_UARTGetCharShouldReturnErrorIfRSRECRErrorFlagIsSet(void)
{
    status_t status = STATUS_OK;
    uint8_t c = 'c';

    set_RSRECR_ERR_flag();
    g_uart.initialized = true;

    status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(status, UART_STATUS_RECV_ERROR);
}

/**
 * Tests if UART get char fails if char pointer is invalid
 */
void test_UARTGetCharShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    g_uart.initialized = true;

    status = uart_getchar(NULL);

    TEST_ASSERT_EQUAL_UINT(status, UART_STATUS_INV_PTR);
}

// ========================================================
// uart_read
// ========================================================

/**
 * Tests if UART read reads from proper register
 */
void test_UARTReadShouldReadDataFromDR(void)
{
    status_t status = STATUS_OK;
    const uint32_t dr = 0xABCD;
    const uint8_t c = (uint8_t)(dr & DR_DATA_MASK);
    uint8_t data[8];

    g_uart.initialized = true;
    g_mock_uart_registers.DR = dr;

    status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EACH_EQUAL_UINT8(c, data, sizeof(data));
}

/**
 * Tests if UART read fails if UART is not initalized
 */
void test_UARTReadShouldFailIfUARTIsNotInitialized(void)
{
    status_t status = STATUS_OK;
    uint8_t data[8];

    g_uart.initialized = false;

    status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINIT, status);
}

/**
 * Tests if UART read fails when data pointer is invalid
 */
void test_UARTReadShouldFailForInvalidPointer(void)
{
    status_t status = STATUS_OK;

    g_uart.initialized = true;

    status = uart_read(NULL, 1);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INV_PTR, status);
}

/**
 * Tests if UART read reads no data when length is 0
 */
void test_UARTReadShouldReadNothingForDataLengthZero(void)
{
    status_t status = STATUS_OK;
    const int8_t initial_data[] = "some data";
    uint8_t data[16];
    memcpy(data, initial_data, sizeof(initial_data));

    g_uart.initialized = true;

    status = uart_read(data, 0);

    TEST_ASSERT_EQUAL_UINT(STATUS_OK, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(initial_data, data, sizeof(initial_data));
}

/**
 * Tests if UART read fails when receive error occurs
 */
void test_UARTReadShouldFailIfReceiveErrorOccurs(void)
{
    status_t status = STATUS_OK;
    uint8_t data[8];

    set_RSRECR_ERR_flag();
    g_uart.initialized = true;

    status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_RECV_ERROR, status);
}

/**
 * Tests if UART read hit timeout when there is no data
 */
void test_UARTReadShouldReturnTimeoutIfNoData(void)
{
    status_t status = STATUS_OK;
    uint8_t data[8];

    set_FR_RXFE_flag();
    g_uart.initialized = true;

    status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_TIMEOUT, status);
}

// ========================================================
// mocks
// ========================================================

void mock_csr_read_callback() { g_mock_csr += TIMER_CLOCK_FREQ >> 4; }

// ========================================================
// helper functions
// ========================================================

static uart_config_t get_valid_uart_config_t()
{
    uart_config_t ret = {.data_bits = 8, .stop_bits = 1, .parity = false, .baudrate = 115200};
    return ret;
}

static void clear_FR_TXFF_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&g_mock_uart_registers.FR;
    *FR_ptr &= ~FR_TXFF;
}

static void clear_FR_RXFE_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&g_mock_uart_registers.FR;
    *FR_ptr &= ~FR_RXFE;
}

static void set_FR_RXFE_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&g_mock_uart_registers.FR;
    *FR_ptr |= FR_RXFE;
}

static void clear_RSRECR_ERR_flag() { g_mock_uart_registers.RSRECR &= ~RSRECR_ERR_MASK; }

static void set_RSRECR_ERR_flag() { g_mock_uart_registers.RSRECR |= RSRECR_ERR_MASK; }
