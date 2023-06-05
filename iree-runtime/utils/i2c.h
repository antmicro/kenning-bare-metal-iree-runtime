/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_I2C_H_
#define IREE_RUNTIME_UTILS_I2C_H_

#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "springbok.h"

/**
 * An enum that describes I2C status
 */
typedef enum
{
    I2C_STATUS_OK = 0,
    I2C_STATUS_UNINITIALIZED,
    I2C_STATUS_INVALID_POINTER,
    I2C_STATUS_INVALID_ARGUMENT,
} I2C_STATUS;

typedef enum
{
    I2C_SPEED_STANDARD,
    I2C_SPEED_FAST,
    I2C_SPEED_FAST_PLUS
} I2C_SPEED;

/**
 * A struct that contains registers used by I2C
 */
typedef volatile struct __attribute__((packed, aligned(4)))
{
    uint32_t INTR_STATE;            /* 0x0 Interrupt State Register */
    uint32_t INTR_ENABLE;           /* 0x4 Interrupt Enable Register */
    uint32_t INTR_TEST;             /* 0x8 Interrupt Test Register */
    uint32_t ALERT_TEST;            /* 0xC Alert Test Register */
    uint32_t CTRL;                  /* 0x10 I2C Control Register */
    uint32_t STATUS;                /* 0x14 I2C Live Status Register */
    uint32_t RDATA;                 /* 0x18 I2C Read Data */
    uint32_t FDATA;                 /* 0x1C I2C Format Data */
    uint32_t FIFO_CTRL;             /* 0x20 I2C FIFO control register */
    uint32_t FIFO_STATUS;           /* 0x24 I2C FIFO status register */
    uint32_t OVRD;                  /* 0x28 I2C Override Control Register */
    uint32_t VAL;                   /* 0x2C Oversampled RX values */
    uint32_t TIMING0;               /* 0x30 Detailed I2C Timings (directly corresponding to table 10 in the I2C */
    uint32_t TIMING1;               /* 0x34 Specification). All values are expressed in units of the input clock */
    uint32_t TIMING2;               /* 0x38 period.*/
    uint32_t TIMING3;               /* 0x3C */
    uint32_t TIMING4;               /* 0x40 */
    uint32_t TIMEOUT_CTRL;          /* 0x44 I2C clock stretching timeout control */
    uint32_t TARGET_ID;             /* 0x48 I2C target address and mask pairs */
    uint32_t ACQDATA;               /* 0x4C I2C target acquired data */
    uint32_t TXDATA;                /* 0x50 I2C target transmit data */
    uint32_t HOST_TIMEOUT_CTRL;     /* 0x54 I2C host clock generation timeout value (in units of input clock */
                                    /* frequency) */
} i2c_registers;

#define CTRL_ENABLEHOST (1) /* Enable Host I2C functionality */
#define CTRL_ENABLETARGET (1 << 1u) /* Enable Target I2C functionality */

#define STATUS_FMTFULL (1) /* FMT FIFO is full */
#define STATUS_RXFULL (1 << 1u) /* RX FIFO is full */

#define RDATA_RDATA_OFFSET (0)
#define RDATA_RDATA_MASK (0xFF << RDATA_RDATA_OFFSET)

#define FIFO_CTRL_RXRST_OFFSET (0)
#define FIFO_CTRL_RXRST_MASK (0x1 << FIFO_CTRL_RXRST_OFFSET)

#define FIFO_CTRL_FMTRST_OFFSET (1)
#define FIFO_CTRL_FMTRST_MASK (0x1 << FIFO_CTRL_FMTRST_OFFSET)

#define FIFO_CTRL_ACQRST_OFFSET (7)
#define FIFO_CTRL_ACQRST_MASK (0x1 << FIFO_CTRL_ACQRST_OFFSET)

#define FIFO_CTRL_TXRST_OFFSET (8)
#define FIFO_CTRL_TXRST_MASK (0x1 << FIFO_CTRL_TXRST_OFFSET)

#define TIMING0_THIGH_OFFSET (16)
#define TIMING0_THIGH_MASK (0xFFFF << TIMING0_THIGH_OFFSET)
#define TIMING0_TLOW_OFFSET (0)
#define TIMING0_TLOW_MASK (0xFFFF << TIMING0_TLOW_OFFSET)

#define TIMING1_T_R_OFFSET (16)
#define TIMING1_T_R_MASK (0xFFFF << TIMING1_T_R_OFFSET)
#define TIMING1_T_F_OFFSET (0)
#define TIMING1_T_F_MASK (0xFFFF << TIMING1_T_F_OFFSET)

#define TIMING2_TSU_STA_OFFSET (16)
#define TIMING2_TSU_STA_MASK (0xFFFF << TIMING2_TSU_STA_OFFSET)
#define TIMING2_THD_STA_OFFSET (0)
#define TIMING2_THD_STA_MASK (0xFFFF << TIMING2_THD_STA_OFFSET)

#define TIMING3_TSU_DAT_OFFSET (16)
#define TIMING3_TSU_DAT_MASK (0xFFFF << TIMING3_TSU_DAT_OFFSET)
#define TIMING3_THD_DAT_OFFSET (0)
#define TIMING3_THD_DAT_MASK (0xFFFF << TIMING3_THD_DAT_OFFSET)

#define TIMING4_TSU_STO_OFFSET (16)
#define TIMING4_TSU_STO_MASK (0xFFFF << TIMING4_TSU_STO_OFFSET)
#define TIMING4_T_BUF_OFFSET (0)
#define TIMING4_T_BUF_MASK (0xFFFF << TIMING4_T_BUF_OFFSET)

#define TARGET_ID_ADDRESS0_OFFEST (0)
#define TARGET_ID_ADDRESS0_MASK (0x7F << TARGET_ID_ADDRESS0_OFFEST)
#define TARGET_ID_MASK0_OFFSET (7)
#define TARGET_ID_MASK0_MASK (0x7F << TARGET_ID_MASK0_OFFSET)
#define TARGET_ID_ADDRESS1_OFFEST (14)
#define TARGET_ID_ADDRESS1_MASK (0x7F << TARGET_ID_ADDRESS1_OFFEST)
#define TARGET_ID_MASK1_OFFSET (21)
#define TARGET_ID_MASK1_MASK (0x7F << TARGET_ID_MASK1_OFFSET)

/**
 * A struct that contains I2C informations
 */
typedef struct
{
    i2c_registers *registers;
    bool initialized;
} i2c_t;

/**
 * A struct that contains I2C config
 */
typedef struct
{
    uint32_t clock_period_nanos;
} i2c_config_t;

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

#ifndef __UNIT_TEST__
#define I2C_ADDRESS (0x40080000) /* address of I2C registers */
#else                            // __UNIT_TEST_
extern i2c_registers g_mock_i2c_registers;
#define I2C_ADDRESS (&g_mock_i2c_registers)
#endif                           // __UNIT_TEST_

/**
 * Initializes I2C
 *
 * @param config I2C configuration
 *
 * @returns status of initialization
 */
I2C_STATUS i2c_init(const i2c_config_t *config);

I2C_STATUS i2c_write_byte(const uint8_t byte);

I2C_STATUS i2c_write(const uint8_t *data, size_t data_length);

I2C_STATUS i2c_read_byte(uint8_t *byte);

I2C_STATUS i2c_read(const uint8_t *data, size_t data_length);

#endif // IREE_RUNTIME_UTILS_I2C_H_
