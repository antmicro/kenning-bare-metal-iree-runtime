#include "uart.h"

#ifndef __UNIT_TEST__
static uart_t g_uart = {.initialized = false};
#else  // __UNIT_TEST__
uart_t g_uart = {.initialized = false};
#endif // __UNIT_TEST__

UART_STATUS uart_init(const uart_config *config)
{
    if (g_uart.initialized)
    {
        return UART_STATUS_OK;
    }
    if (!IS_VALID_POINTER(config))
    {
        return UART_STATUS_INVALID_POINTER;
    }

    g_uart.registers = (uart_registers *)UART_ADDRESS;

    if (config->data_bits < 5u || config->data_bits > 8u)
    {
        return UART_STATUS_INVALID_ARGUMENT_WORDSIZE;
    }
    if (config->stop_bits == 0u || config->stop_bits > 2u)
    {
        return UART_STATUS_INVALID_ARGUMENT_STOP_BITS;
    }
#define CHECK_UART(baudrate_value) (baudrate_value == config->baudrate) ||
    if (!(BAUDRATE_VALUES(CHECK_UART) false))
    {
        return UART_STATUS_INVALID_ARGUMENT_BAUDRATE;
    }
#undef CHECK_UART

    g_uart.registers->CR &= ~CR_UARTEN;
    while (g_uart.registers->FR & FR_BUSY)
    {
    }
    g_uart.registers->LCRH &= ~LCRH_FEN;

    double intpart = 0;
    double fractpart = 0;
    double baudrate_divisor = (double)REF_CLOCK / (16u * config->baudrate);
    fractpart = modf(baudrate_divisor, &intpart);

    g_uart.registers->IBRD = (uint16_t)intpart;
    g_uart.registers->FBRD = (uint8_t)((fractpart * 64u) + 0.5);

    uint32_t lcrh = 0u;

    switch (config->data_bits)
    {
    case 5:
        lcrh |= LCRH_WLEN_5BITS;
        break;
    case 6:
        lcrh |= LCRH_WLEN_6BITS;
        break;
    case 7:
        lcrh |= LCRH_WLEN_7BITS;
        break;
    case 8:
        lcrh |= LCRH_WLEN_8BITS;
        break;
    }

    if (config->parity)
    {
        lcrh |= LCRH_PEN;
        lcrh |= LCRH_EPS;
        lcrh |= LCRH_SPS;
    }
    else
    {
        lcrh &= ~LCRH_PEN;
        lcrh &= ~LCRH_EPS;
        lcrh &= ~LCRH_SPS;
    }

    if (config->stop_bits == 1u)
    {
        lcrh &= ~LCRH_STP2;
    }
    else if (config->stop_bits == 2u)
    {
        lcrh |= LCRH_STP2;
    }

    lcrh |= LCRH_FEN;

    g_uart.registers->LCRH = lcrh;

    g_uart.registers->CR |= CR_UARTEN;

    g_uart.initialized = true;

    return UART_STATUS_OK;
}

UART_STATUS uart_putchar(const uint8_t c)
{
    if (!g_uart.initialized)
    {
        return UART_STATUS_UNINITIALIZED;
    }
    while (g_uart.registers->FR & FR_TXFF)
    {
    }
    g_uart.registers->DR = c;

    return UART_STATUS_OK;
}

UART_STATUS uart_write(const uint8_t *data, size_t data_length)
{
    if (!g_uart.initialized)
    {
        return UART_STATUS_UNINITIALIZED;
    }
    if (!IS_VALID_POINTER(data))
    {
        return UART_STATUS_INVALID_POINTER;
    }

    UART_STATUS status = UART_STATUS_OK;

    for (int i = 0; i < data_length; ++i)
    {
        uart_putchar(data[i]);
        if (UART_STATUS_OK != status)
        {
            return status;
        }
    }
    return status;
}

UART_STATUS uart_getchar(uint8_t *c)
{
    if (!IS_VALID_POINTER(c))
    {
        return UART_STATUS_INVALID_POINTER;
    }
    if (!g_uart.initialized)
    {
        return UART_STATUS_UNINITIALIZED;
    }
    if (g_uart.registers->FR & FR_RXFE)
    {
        return UART_STATUS_NO_DATA;
    }

    *c = g_uart.registers->DR & DR_DATA_MASK;
    if (g_uart.registers->RSRECR & RSRECR_ERR_MASK)
    {
        g_uart.registers->RSRECR &= RSRECR_ERR_MASK;
        return UART_STATUS_RECEIVE_ERROR;
    }
    return UART_STATUS_OK;
}

UART_STATUS uart_read(uint8_t *data, size_t data_length)
{
    int i = 0;
    register uint32_t start_timer;
    register uint32_t end_timer;

    if (!IS_VALID_POINTER(data))
    {
        return UART_STATUS_INVALID_POINTER;
    }
    if (!g_uart.initialized)
    {
        return UART_STATUS_UNINITIALIZED;
    }

    CSR_READ(start_timer, CSR_TIME);
    while (i < data_length)
    {
        uint8_t c = 0;
        UART_STATUS status = uart_getchar(&c);
        CSR_READ(end_timer, CSR_TIME);
        if (UART_STATUS_OK == status)
        {
            CSR_READ(start_timer, CSR_TIME);
            data[i] = c;
            ++i;
        }
        else if (UART_STATUS_RECEIVE_ERROR == status)
        {
            return UART_STATUS_RECEIVE_ERROR;
        }
        else if (end_timer - start_timer > UART_TIMEOUT_S * TIMER_CLOCK_FREQ)
        {
            return UART_STATUS_TIMEOUT;
        }
    }
    return UART_STATUS_OK;
}
