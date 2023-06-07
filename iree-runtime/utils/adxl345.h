/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_ADXL345_H_
#define IREE_RUNTIME_UTILS_ADXL345_H_

#include "i2c.h"
#include "utils.h"

/**
 * ADXL345 custom error codes
 */
#define ADXL345_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(ADXL345);

#define ADXL345_I2C_ADDRESS (0x1D)

#define ADXL345_DEVICE_ID (0xE5)
#define ADXL345_DATA_X0 (0x32)

#define ADXL345_DATA_FORMAT (0x31)
#define ADXL345_DATA_FORMAT_RANGE_BITS (0x3 << 0)

#define ADXL345_BUFFER_LEN (128)

#define ADXL345_READ_INTERVAL (0.01F) // 10 ms (100 Hz)

/**
 * A struct that contains acceleration data read from ADXL345
 */
typedef struct __attribute__((packed))
{
    float x;
    float y;
    float z;
} adxl345_data_t;

/**
 * Reads data from ADXL345 sensor at given address
 *
 * @param address I2C address of the sensor
 * @param data output data buffer
 *
 * @returns status of the sensor
 */
status_t adxl345_read_data(adxl345_data_t *data);

#endif // IREE_RUNTIME_UTILS_ADXL345_H_
