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

/**
 * An enum that describes I2C status
 */
typedef enum
{
    I2C_STATUS_OK = 0,
    I2C_STATUS_RECEIVE_ERROR,
    I2C_STATUS_NO_DATA,
    I2C_STATUS_UNINITIALIZED,
    I2C_STATUS_INVALID_POINTER,
} I2C_STATUS;

/**
 * A struct that contains registers used by I2C
 */
typedef volatile struct __attribute__((packed, aligned(4)))
{
    uint32_t interruptState;
    uint32_t interruptEnable;
    uint32_t interruptTest;
    uint32_t alertTest;
    uint32_t CTRL;                  /* I2C Control Register */
    uint32_t STATUS;                /* I2C Live Status Register */
    uint32_t RDATA;                 /* I2C Read Data */
    uint32_t FDATA;                 /* I2C Format Data */
    uint32_t FIFO_CTRL;             /* I2C FIFO control register */
    uint32_t FIFO_STATUS;           /* I2C FIFO status register */
    uint32_t OVRD;                  /* I2C Override Control Register */
    uint32_t VAL;                   /* Oversampled RX values */
    uint32_t TIMING0;               /* Detailed I2C Timings (directly corresponding to table 10 in the I2C */
    uint32_t TIMING1;               /* Specification). All values are expressed in units of the input clock period.*/
    uint32_t TIMING2;
    uint32_t TIMING3;
    uint32_t TIMING4;
    uint32_t TIMEOUT_CTRL;          /* I2C clock stretching timeout control */
    uint32_t TARGET_ID;             /* I2C target address and mask pairs */
    uint32_t ACQDATA;               /* I2C target acquired data */
    uint32_t TXDATA;                /* I2C target transmit data */
    uint32_t HOST_TIMEOUT_CTRL;     /* I2C host clock generation timeout value (in units of input clock frequency) */

} i2c_registers;

/**
 * A struct that contains I2C informations
 */
typedef struct
{
    i2c_registers *registers;
    bool initialized;
} i2c_t;

/**
 * A struct that contains UART config
 */
typedef struct
{

} i2c_config;

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
I2C_STATUS i2c_init(const i2c_config *config);

#endif // IREE_RUNTIME_UTILS_I2C_H_