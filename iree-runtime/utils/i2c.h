/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_I2C_H_
#define IREE_RUNTIME_UTILS_I2C_H_

#include "utils.h"
#include <stdbool.h>
#include <string.h>

/**
 * I2C custom error codes
 */
#define I2C_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(I2C);

/**
 * An enum with I2C speeds
 */
typedef enum
{
    I2C_SPEED_STANDARD,
    I2C_SPEED_FAST,
    I2C_SPEED_FAST_PLUS
} I2C_SPEED;

/**
 * An enum with I2C formats for FDATA register
 */
typedef enum
{
    I2C_FORMAT_START,
    I2C_FORMAT_TX,
    I2C_FORMAT_TX_STOP,
    I2C_FORMAT_RX,
    I2C_FORMAT_RX_CONT,
    I2C_FORMAT_RX_STOP
} I2C_FORMAT;

/**
 * A struct that contains registers used by I2C
 */
typedef volatile struct __attribute__((packed, aligned(4)))
{
    uint32_t INTR_STATE;        /* 0x0 Interrupt State Register */
    uint32_t INTR_ENABLE;       /* 0x4 Interrupt Enable Register */
    uint32_t INTR_TEST;         /* 0x8 Interrupt Test Register */
    uint32_t ALERT_TEST;        /* 0xC Alert Test Register */
    uint32_t CTRL;              /* 0x10 I2C Control Register */
    uint32_t STATUS;            /* 0x14 I2C Live Status Register */
    uint32_t RDATA;             /* 0x18 I2C Read Data */
    uint32_t FDATA;             /* 0x1C I2C Format Data */
    uint32_t FIFO_CTRL;         /* 0x20 I2C FIFO control register */
    uint32_t FIFO_STATUS;       /* 0x24 I2C FIFO status register */
    uint32_t OVRD;              /* 0x28 I2C Override Control Register */
    uint32_t VAL;               /* 0x2C Oversampled RX values */
    uint32_t TIMING0;           /* 0x30 Detailed I2C Timings (directly corresponding to table 10 in the I2C */
    uint32_t TIMING1;           /* 0x34 Specification). All values are expressed in units of the input clock */
    uint32_t TIMING2;           /* 0x38 period.*/
    uint32_t TIMING3;           /* 0x3C */
    uint32_t TIMING4;           /* 0x40 */
    uint32_t TIMEOUT_CTRL;      /* 0x44 I2C clock stretching timeout control */
    uint32_t TARGET_ID;         /* 0x48 I2C target address and mask pairs */
    uint32_t ACQDATA;           /* 0x4C I2C target acquired data */
    uint32_t TXDATA;            /* 0x50 I2C target transmit data */
    uint32_t HOST_TIMEOUT_CTRL; /* 0x54 I2C host clock generation timeout value (in units of input clock */
                                /* frequency) */
} i2c_registers_t;

/**
 * CTRL register fields
 */
#define CTRL_ENABLEHOST (1)         /* Enable Host I2C functionality */
#define CTRL_ENABLETARGET (1 << 1u) /* Enable Target I2C functionality */

/**
 * STATUS register fields
 */
#define STATUS_FMT_FULL (0x1 << 0)    /* FMT FIFO is full */
#define STATUS_RX_FULL (0x1 << 1)     /* RX FIFO is full */
#define STATUS_FMT_EMPTY (0x1 << 2)   /* FMT FIFO is empty */
#define STATUS_HOST_IDLE (0x1 << 3)   /* Host functionality is idle. No Host transaction is in progress */
#define STATUS_TARGET_IDLE (0x1 << 4) /* Target functionality is idle. No Target transaction is in progress */
#define STATUS_RX_EMPTY (0x1 << 5)    /* RX FIFO is empty */
#define STATUS_TX_FULL (0x1 << 6)     /* TX FIFO is full */
#define STATUS_ACQ_FULL (0x1 << 7)    /* ACQ FIFO is full */
#define STATUS_TX_EMPTY (0x1 << 8)    /* TX FIFO is empty */
#define STATUS_ACQ_EMPTY (0x1 << 9)   /* ACQ FIFO is empty */

/**
 * RDATA register fields
 */
#define RDATA_RDATA (0xFF << 0)

/**
 * FDATA register fields
 */
#define FDATA_FBYTE (0xFF << 0) /* Format Byte. Directly transmitted if no flags are set */
#define FDATA_START (0x1 << 8)  /* Issue a START condition before transmitting BYTE */
#define FDATA_STOP (0x1 << 9)   /* Issue a STOP condition after this operation */
#define FDATA_READ (0x1 << 10)  /* Read BYTE bytes from I2C. (256 if BYTE==0) */
#define FDATA_RCONT (0x1 << 11) /* Do not NACK the last byte read, let the read operation continue */
#define FDATA_NAKOK (0x1 << 12) /* Do not signal an exception if the current byte is not ACK’d */

/**
 * FIFO_CTRL register fields
 */
#define FIFO_CTRL_RXRST (0x1 << 0)  /* RX fifo reset. Write 1 to the register resets RX_FIFO. Read returns 0 */
#define FIFO_CTRL_FMTRST (0x1 << 1) /* FMT fifo reset. Write 1 to the register resets FMT_FIFO. Read returns 0 */
#define FIFO_CTRL_ACQRST (0x1 << 7) /* ACQ FIFO reset. Write 1 to the register resets it. Read returns 0 */
#define FIFO_CTRL_TXRST (0x1 << 8)  /* TX FIFO reset. Write 1 to the register resets it. Read returns 0 */

/**
 * TIMING0 register fields
 */
#define TIMING0_THIGH (0xFFFF << 16) /* The actual time to hold SCL high in a given pulse */
#define TIMING0_TLOW (0xFFFF << 0)   /* The actual time to hold SCL low between any two SCL pulses */

/**
 * TIMING1 register fields
 */
#define TIMING1_T_R (0xFFFF << 16) /* The nominal rise time to anticipate for the bus */
#define TIMING1_T_F (0xFFFF << 0)  /* The nominal fall time to anticipate for the bus */

/**
 * TIMING2 register fields
 */
#define TIMING2_TSU_STA (0xFFFF << 16) /* Actual setup time for repeated start signals */
#define TIMING2_THD_STA (0xFFFF << 0)  /* Actual hold time for start signals */

/**
 * TIMING3 register fields
 */
#define TIMING3_TSU_DAT (0xFFFF << 16) /* Actual setup time for data (or ack) bits */
#define TIMING3_THD_DAT (0xFFFF << 0)  /* Actual hold time for data (or ack) bits */

/*
 * TIMING4 register fields
 */
#define TIMING4_TSU_STO (0xFFFF << 16) /* Actual setup time for stop signals */
#define TIMING4_T_BUF (0xFFFF << 0)    /* Actual time between each STOP signal and the following START signal */

/**
 * A struct that contains I2C informations
 */
typedef struct
{
    i2c_registers_t *registers;
    bool initialized;
} i2c_t;

/**
 * A struct that contains I2C config
 */
typedef struct
{
    I2C_SPEED speed;
    uint32_t clock_period_nanos;
} i2c_config_t;

/**
 * A struct that contains I2C status flags
 */
typedef struct
{
    bool fmt_full;    /* FMT FIFO is full */
    bool rx_full;     /* RX FIFO is full */
    bool fmt_empty;   /* FMT FIFO is empty */
    bool host_idle;   /* Host functionality is idle. No Host transaction is in progress */
    bool target_idle; /* Target functionality is idle. No Target transaction is in progress */
    bool rx_empty;    /* RX FIFO is empty */
    bool tx_full;     /* TX FIFO is full */
    bool acq_full;    /* ACQ FIFO is full */
    bool tx_empty;    /* TX FIFO is empty */
    bool acq_empty;   /* ACQ FIFO is empty */
} i2c_status_t;

/**
 * A struct that contains I2C config
 */
typedef struct
{
    uint16_t scl_time_high_cycles;
    uint16_t scl_time_low_cycles;
    uint16_t rise_cycles;
    uint16_t fall_cycles;
    uint16_t start_signal_setup_cycles;
    uint16_t start_signal_hold_cycles;
    uint16_t data_signal_setup_cycles;
    uint16_t data_signal_hold_cycles;
    uint16_t stop_signal_setup_cycles;
    uint16_t stop_signal_hold_cycles;
} i2c_timing_config_t;

/**
 * A struct that contains format flags
 */
typedef struct
{
    bool start;
    bool stop;
    bool read;
    bool read_cont;
    bool no_ack_ok;
} i2c_format_flags_t;

#ifndef __UNIT_TEST__
#define I2C_ADDRESS (0x40080000) /* address of I2C registers */
#else                            // __UNIT_TEST_
extern i2c_registers_t g_mock_i2c_registers;
#define I2C_ADDRESS (&g_mock_i2c_registers)
#endif // __UNIT_TEST_

/**
 * Reads I2C status
 *
 * @param status output status
 *
 * @returns status of the I2C
 */
status_t i2c_get_status(i2c_status_t *status);

/**
 * Writes single byte to I2C
 *
 * @param byte data
 * @param format data format
 *
 * @returns status of the I2C
 */
status_t i2c_write_byte(const uint8_t byte, I2C_FORMAT format);

/**
 * Reads single byte from I2C
 *
 * @param byte output data
 *
 * @returns status of the I2C
 */
status_t i2c_read_byte(uint8_t *byte);

/**
 * Writes single target register
 *
 * @param target_id I2C address of the target
 * @param address register address
 * @param data data to be written
 *
 * @returns status of the I2C
 */
status_t i2c_write_target_register(uint8_t target_id, uint8_t address, uint8_t data);

/**
 * Reads single target register
 *
 * @param target_id I2C address of the target
 * @param address register address
 * @param data output data buffer
 *
 * @returns status of the I2C
 */
status_t i2c_read_target_register(uint8_t target_id, uint8_t address, uint8_t *data);

/**
 * Reads multiple target register
 *
 * @param target_id I2C address of the target
 * @param address register address
 * @param count number of registers to be read
 * @param data output data buffer
 *
 * @returns status of the I2C
 */
status_t i2c_read_target_registers(uint8_t target_id, uint8_t address, size_t count, uint8_t *data);

#endif // IREE_RUNTIME_UTILS_I2C_H_
