/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UNIT_TESTS_SENSOR_MOCK_H_
#define IREE_RUNTIME_UNIT_TESTS_SENSOR_MOCK_H_

#include "utils.h"

/**
 * Sensor mock custom error codes
 */
#define SENSOR_MOCK_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(SENSOR_MOCK);

#define SENSOR_MOCK_I2C_ADDRESS (0x1A)

#define SENSOR_MOCK_DEVICE_ID (0xAB)
#define SENSOR_MOCK_DATA_ADDRESS (0xCD)

#define SENSOR_MOCK_BUFFER_LEN (32)
#define SENSOR_MOCK_READ_INTERVAL (0.01F) // 10 ms (100 Hz)

typedef struct __attribute__((packed))
{
    float a;
    float b;
} sensor_mock_data_t;

status_t sensor_mock_read_data(sensor_mock_data_t *data);

#endif // IREE_RUNTIME_UNIT_TESTS_SENSOR_MOCK_H_
