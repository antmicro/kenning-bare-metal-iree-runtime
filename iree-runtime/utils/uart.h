#ifndef IREE_RUNTIME_UTILS_UART_H_
#define IREE_RUNTIME_UTILS_UART_H_

#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * An enum that describes UART status
 */
typedef enum
{
    UART_STATUS_OK = 0,
    UART_STATUS_INVALID_ARGUMENT_BAUDRATE,
    UART_STATUS_INVALID_ARGUMENT_WORDSIZE,
    UART_STATUS_INVALID_ARGUMENT_STOP_BITS,
    UART_STATUS_RECEIVE_ERROR,
    UART_STATUS_NO_DATA,
    UART_STATUS_TIMEOUT,
} UART_STATUS;

/**
 * A struct that contains registers used by UART
 */
typedef volatile struct __attribute__((packed, aligned(4)))
{
    uint32_t DR;            /* 0x0 Data Register */
    uint32_t RSRECR;        /* 0x4 Receive status / error clear register */
    uint32_t _reserved0[4]; /* 0x8 - 0x14 reserved */
    const uint32_t FR;      /* 0x18 Flag register */
    uint32_t _reserved1;    /* 0x1C reserved */
    uint32_t ILPR;          /* 0x20 Low-power counter register */
    uint32_t IBRD;          /* 0x24 Integer baudrate register */
    uint32_t FBRD;          /* 0x28 Fractional baudrate register */
    uint32_t LCRH;          /* 0x2C Line control register */
    uint32_t CR;            /* 0x30 Control register */
} uart_registers;

/**
 * A struct that contains UART informations
 */
typedef struct
{
    uart_registers *registers;
    bool initialized;
} uart_t;

/**
 * A struct that contains UART config
 */
typedef struct
{
    uint8_t data_bits;
    uint8_t stop_bits;
    bool parity;
    uint32_t baudrate;
} uart_config;

#define UART_TIMEOUT_S (1) /* UART read timeout (1 second) */

#define UART_ADDRESS (0x40000000) /* address of UART registers */
#define REF_CLOCK (24000000u)     /* UART reference clock (24 MHz) */

#define DR_DATA_MASK (0xFFu) /* mask of the data register */

#define FR_BUSY (1 << 3u) /* busy flag */
#define FR_RXFE (1 << 4u) /* receive FIFO empty flag  */
#define FR_TXFF (1 << 5u) /* transmit FIFO full flag  */

#define RSRECR_ERR_MASK (0xFu) /* receive status error mask */

#define LCRH_FEN (1 << 4u)  /* FIFO enable */
#define LCRH_PEN (1 << 1u)  /* parity enable */
#define LCRH_EPS (1 << 2u)  /* even parity select */
#define LCRH_STP2 (1 << 3u) /* two stop bits select */
#define LCRH_SPS (1 << 7u)  /* stick parity select */
#define CR_UARTEN (1 << 0u) /* UART enable */

#define LCRH_WLEN_5BITS (0u << 5u) /* 5 bit word length */
#define LCRH_WLEN_6BITS (1u << 5u) /* 6 bit word length */
#define LCRH_WLEN_7BITS (2u << 5u) /* 7 bit word length */
#define LCRH_WLEN_8BITS (3u << 5u) /* 8 bit word length */

/**
 * Initializes UART
 *
 * @param config UART configuration
 *
 * @returns status of initialization
 */
UART_STATUS uart_init(const uart_config *config);
/**
 * Writes single byte to UART
 *
 * @param c byte to be written
 */
void uart_putchar(uint8_t c);
/**
 * Write buffer of bytes to UART
 *
 * @param data buffer to be written
 * @param data_length length of the buffer
 */
void uart_write(const uint8_t *data, size_t data_length);
/**
 * Reads single byte from UART
 *
 * @param c read byte
 *
 * @returns status of read action
 */
UART_STATUS uart_getchar(uint8_t *c);
/**
 * Reads bytes from UART into given buffer
 *
 * @param data buffer for results
 * @param data_length length of the buffer
 *
 * @returns status of read action
 */
UART_STATUS uart_read(uint8_t *data, size_t data_length);

#endif // IREE_RUNTIME_UTILS_UART_H_