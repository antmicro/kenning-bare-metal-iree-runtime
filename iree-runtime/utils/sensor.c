/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensor.h"

GENERATE_MODULE_STATUSES_STR(SENSOR);

ut_static sensor_data_t g_sensor_data_buffer[SENSOR_BUFFER_LEN];
ut_static size_t g_sensor_data_buffer_idx = 0;
static uint32_t g_sensor_last_read_time = 0;

status_t sensor_init()
{
    status_t status = STATUS_OK;
    uint8_t device_id = 0;
    uint8_t valid_device_id = SENSOR_DEVICE_ID;

    // validate device ID
    status = sensor_get_device_id(&device_id);
    RETURN_ON_ERROR(status, status);

    if (device_id != valid_device_id)
    {
        LOG_ERROR("Invalid sensor device ID: 0x%x. Expected: 0x%x", device_id, valid_device_id);
        return SENSOR_STATUS_INV_SENSOR;
    }

    return STATUS_OK;
}

status_t sensor_get_device_id(uint8_t *device_id)
{
    VALIDATE_POINTER(device_id, SENSOR_STATUS_INV_PTR);

    return i2c_read_target_register(SENSOR_I2C_ADDRESS, 0x0, device_id);
}

status_t sensor_get_data_size(size_t *data_size)
{
    VALIDATE_POINTER(data_size, SENSOR_STATUS_INV_PTR);

    *data_size = sizeof(sensor_data_t);

    return STATUS_OK;
}

status_t sensor_read_data_into_buffer()
{
    status_t status = STATUS_OK;
    sensor_data_t sensor_data = {0};

    LOG_DEBUG("Reading sensor data into buffer. Buffer idx: %d", g_sensor_data_buffer_idx);

    status_t (*read_data_function)(sensor_data_t *) = SENSOR_READ_DATA_FUN;

    register uint32_t timer;

    do
    {
        CSR_READ(timer, CSR_TIME);
    } while (timer - g_sensor_last_read_time < (int)(SENSOR_READ_INTERVAL * TIMER_CLOCK_FREQ));
    g_sensor_last_read_time = timer;

    // read data
    status = read_data_function(&sensor_data);
    RETURN_ON_ERROR(status, status);

    // write data to the buffer
    g_sensor_data_buffer[g_sensor_data_buffer_idx] = sensor_data;

    // increment buffer index
    ++g_sensor_data_buffer_idx;
    g_sensor_data_buffer_idx %= SENSOR_BUFFER_LEN;

    return status;
}

status_t sensor_get_buffered_data(size_t output_size, uint8_t *output)
{
    VALIDATE_POINTER(output, SENSOR_STATUS_INV_PTR);

    if (SENSOR_BUFFER_LEN * sizeof(sensor_data_t) != output_size)
    {
        return SENSOR_STATUS_INV_ARG;
    }

    // as we don't shift buffer during read we need to copy the sensor data so that the indices of buffer are mapped
    // like [0, ..., len - 1] -> [g_sensor_data_buffer_idx, ..., len - 1, 0, ..., g_sensor_data_buffer_idx - 1]
    memcpy(output, &g_sensor_data_buffer[g_sensor_data_buffer_idx], SENSOR_BUFFER_LEN - g_sensor_data_buffer_idx);
    if (g_sensor_data_buffer_idx > 0)
    {
        memcpy(&output[g_sensor_data_buffer_idx], g_sensor_data_buffer, g_sensor_data_buffer_idx);
    }

    return STATUS_OK;
}