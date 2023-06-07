/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_SENSOR_H_
#define IREE_RUNTIME_UTILS_SENSOR_H_

#if !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))
#include "springbok.h"
#else // !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))
#include "mocks/springbok.h"
#endif // !(defined(__UNIT_TEST__) || defined(__CLANG_TIDY__))

#include "i2c.h"

#if defined(__UNIT_TEST__)
#include "mocks/sensor_mock.h"
#elif defined(I2C_ADXL345)
#include "adxl345.h"
#elif defined(I2C_SENSOR)
#error "No sensor defined"
#endif

#include "utils.h"

/**
 * Selected sensor
 */
#if defined(__UNIT_TEST__) // sensor mock

#define SENSOR_I2C_ADDRESS SENSOR_MOCK_I2C_ADDRESS
#define SENSOR_DEVICE_ID SENSOR_MOCK_DEVICE_ID
#define SENSOR_BUFFER_LEN SENSOR_MOCK_BUFFER_LEN
#define SENSOR_READ_INTERVAL SENSOR_MOCK_READ_INTERVAL
typedef sensor_mock_data_t sensor_data_t;
#define SENSOR_READ_DATA_FUN sensor_mock_read_data

#elif defined(I2C_ADXL345) // adxl345 accelerometer

#define SENSOR_I2C_ADDRESS ADXL345_I2C_ADDRESS
#define SENSOR_DEVICE_ID ADXL345_DEVICE_ID
#define SENSOR_BUFFER_LEN ADXL345_BUFFER_LEN
#define SENSOR_READ_INTERVAL ADXL345_READ_INTERVAL
typedef adxl345_data_t sensor_data_t;
#define SENSOR_READ_DATA_FUN adxl345_read_data

#endif

/**
 * Sensor custom error codes
 */
#define SENSOR_STATUSES(STATUS)      \
    STATUS(SENSOR_STATUS_INV_SENSOR) \
    STATUS(SENSOR_STATUS_NO_SENSOR_AVAILABLE)

GENERATE_MODULE_STATUSES(SENSOR);

/**
 * Retrieves device ID from the sensor
 *
 * @param device_id retrieved device ID
 *
 * @returns status of the sensor
 */
status_t sensor_get_device_id(uint8_t *device_id);

/**
 * Retrieves single data sample size
 *
 * @param data_size single data sample size
 *
 * @returns status of the sensor
 */
status_t sensor_get_data_size(size_t *data_size);

/**
 * Reads data from the sensor into buffer
 *
 * @returns status of the sensor
 */
status_t sensor_read_data_into_buffer();

/**
 * Writes buffered sensor data into provided buffer
 *
 * @param output_size size of the output buffer
 * @param output output buffer
 *
 * @returns status of the sensor
 */
status_t sensor_get_buffered_data(size_t output_size, uint8_t *output);

#endif // IREE_RUNTIME_UTILS_SENSOR_H_
