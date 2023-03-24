#include "../iree-runtime/utils/uart.h"
#include "unity.h"

#include <string.h>

uart_registers mock_uart_registers;
uint32_t mock_csr = 0;
extern uart_t g_uart;

void mock_csr_read_callback();
static uart_config get_valid_uart_config();
static void clear_FR_TXFF_flag();
static void clear_FR_RXFE_flag();
static void set_FR_RXFE_flag();
static void clear_RSRECR_ERR_flag();
static void set_RSRECR_ERR_flag();

void setUp(void)
{
    clear_FR_RXFE_flag();
    clear_FR_TXFF_flag();
    clear_RSRECR_ERR_flag();
}

void tearDown(void) {}

void test_UARTInitSouldSucceedForValidConfig(void)
{

    uart_config config = get_valid_uart_config();
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = false;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_TRUE(g_uart.initialized);
}

void test_UARTInitShouldFailForInvalidConfigPointer(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = false;

    uart_status = uart_init(NULL);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_POINTER, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

void test_UARTInitSouldFailForInvalidDataBitsInConfig(void)
{

    uart_config config = get_valid_uart_config();
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = false;
    config.data_bits = 4;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_WORDSIZE, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);

    g_uart.initialized = false;
    config.data_bits = 9;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_WORDSIZE, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);

    g_uart.initialized = false;
    config.data_bits = 100;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_WORDSIZE, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

void test_UARTInitSouldFailForInvalidStopBitsInConfig(void)
{

    uart_config config = get_valid_uart_config();
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = false;
    config.stop_bits = 0;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_STOP_BITS, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);

    g_uart.initialized = false;
    config.stop_bits = 3;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_STOP_BITS, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

void test_UARTInitShouldFailForInvalidBaudrateConfig(void)
{
    uart_config config = get_valid_uart_config();
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = false;
    config.baudrate = 0u;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_BAUDRATE, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);

    g_uart.initialized = false;
    config.baudrate = 500000u;

    uart_status = uart_init(&config);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_ARGUMENT_BAUDRATE, uart_status);
    TEST_ASSERT_FALSE(g_uart.initialized);
}

void test_UARTPutCharShouldWriteToDataRegister(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint8_t c = 'x';

    g_uart.initialized = true;

    uart_status = uart_putchar(c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EQUAL_UINT8(c, mock_uart_registers.DR & DR_DATA_MASK);
}

void test_UARTPutCharShouldFailAndDontWriteToDRIfUARTIsNotInitialized(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint8_t c = 'c';
    const uint32_t dr = 0xABCD;

    g_uart.initialized = false;
    mock_uart_registers.DR = dr;

    uart_status = uart_putchar(c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINITIALIZED, uart_status);
    TEST_ASSERT_EQUAL_UINT32(dr, mock_uart_registers.DR);
}

void test_UARTWriteShouldWriteStringToDataRegister(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint8_t data[] = "some data";

    g_uart.initialized = true;

    uart_status = uart_write(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EQUAL_UINT8(data[sizeof(data) - 1], mock_uart_registers.DR & DR_DATA_MASK);
}

void test_UARTWriteShouldFailIfDataPointerIsInvalid(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = true;

    uart_status = uart_write(NULL, 1);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_POINTER, uart_status);
}

void test_UARTWriteShouldSucceedAndDontWriteToDRForNoData(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint8_t data[] = "";
    const uint32_t dr = 0xABCD;

    g_uart.initialized = true;
    mock_uart_registers.DR = dr;

    uart_status = uart_write(data, 0);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EQUAL_UINT32(dr, mock_uart_registers.DR);
}

void test_UARTWriteShouldFailAndDontWriteToDRIfUARTIsNotInitialized(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint8_t data[] = "some data";
    const uint32_t dr = 0xABCD;

    g_uart.initialized = false;
    mock_uart_registers.DR = dr;

    uart_status = uart_write(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINITIALIZED, uart_status);
    TEST_ASSERT_EQUAL_UINT32(dr, mock_uart_registers.DR);
}

void test_UARTGetCharShouldReadDataFromDR(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c = 'c';

    g_uart.initialized = true;

    uart_status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EQUAL_UINT8(dr & DR_DATA_MASK, c);
}

void test_UARTGetCharShouldFailIfUARTIsNotInitialized(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    g_uart.initialized = false;

    uart_status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINITIALIZED, uart_status);
}

void test_UARTGetCharShouldReturnNoDataIfRXFEFlagIsSet(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    set_FR_RXFE_flag();
    g_uart.initialized = true;

    uart_status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_NO_DATA, uart_status);
}

void test_UARTGetCharShouldFailWhenUARTIsNotInitializedAndRXFEFlagIsSet(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint32_t dr = 0xABCD;
    uint8_t c;

    set_FR_RXFE_flag();
    g_uart.initialized = false;

    uart_status = uart_getchar(&c);

    TEST_ASSERT_TRUE((uart_status & UART_STATUS_UNINITIALIZED) | (uart_status | UART_STATUS_NO_DATA));
}

void test_UARTGetCharShouldReturnErrorIfRSRECRErrorFlagIsSet(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    uint8_t c = 'c';

    set_RSRECR_ERR_flag();
    g_uart.initialized = true;

    uart_status = uart_getchar(&c);

    TEST_ASSERT_EQUAL_UINT(uart_status, UART_STATUS_RECEIVE_ERROR);
}

void test_UARTGetCharShouldFailForInvalidPointer(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = true;

    uart_status = uart_getchar(NULL);

    TEST_ASSERT_EQUAL_UINT(uart_status, UART_STATUS_INVALID_POINTER);
}

void test_UARTReadShouldReadDataFromDR(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const uint32_t dr = 0xABCD;
    const uint8_t c = (uint8_t)(dr & DR_DATA_MASK);
    uint8_t data[8];

    g_uart.initialized = true;

    uart_status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EACH_EQUAL_UINT8(c, data, sizeof(data));
}

void test_UARTReadShouldFailIfUARTIsNotInitialized(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    uint8_t data[8];

    g_uart.initialized = false;

    uart_status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_UNINITIALIZED, uart_status);
}

void test_UARTReadShouldFailForInvalidPointer(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;

    g_uart.initialized = true;

    uart_status = uart_read(NULL, 1);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_INVALID_POINTER, uart_status);
}

void test_UARTReadShouldReadNothingForDataLengthZero(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    const int8_t initial_data[] = "some data";
    uint8_t data[16];
    memcpy(data, initial_data, sizeof(initial_data));

    g_uart.initialized = true;

    uart_status = uart_read(data, 0);

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_OK, uart_status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(initial_data, data, sizeof(initial_data));
}

void test_UARTReadShouldFailIfReceiveErrorOccurs(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    uint8_t data[8];

    set_RSRECR_ERR_flag();
    g_uart.initialized = true;

    uart_status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_RECEIVE_ERROR, uart_status);
}

void test_UARTReadShouldReturnTimeoutIfNoData(void)
{
    UART_STATUS uart_status = UART_STATUS_OK;
    uint8_t data[8];

    set_FR_RXFE_flag();
    g_uart.initialized = true;

    uart_status = uart_read(data, sizeof(data));

    TEST_ASSERT_EQUAL_UINT(UART_STATUS_TIMEOUT, uart_status);
}

void mock_csr_read_callback() { mock_csr += TIMER_CLOCK_FREQ >> 4; }

static uart_config get_valid_uart_config()
{
    uart_config ret = {.data_bits = 8, .stop_bits = 1, .parity = false, .baudrate = 115200};
    return ret;
}

static void clear_FR_TXFF_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&mock_uart_registers.FR;
    *FR_ptr &= ~FR_TXFF;
}

static void clear_FR_RXFE_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&mock_uart_registers.FR;
    *FR_ptr &= ~FR_RXFE;
}

static void set_FR_RXFE_flag()
{
    uint32_t *FR_ptr = (uint32_t *)&mock_uart_registers.FR;
    *FR_ptr |= FR_RXFE;
}

static void clear_RSRECR_ERR_flag() { mock_uart_registers.RSRECR &= ~RSRECR_ERR_MASK; }

static void set_RSRECR_ERR_flag() { mock_uart_registers.RSRECR |= RSRECR_ERR_MASK; }