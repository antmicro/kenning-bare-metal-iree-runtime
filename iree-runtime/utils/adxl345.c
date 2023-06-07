/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "adxl345.h"

GENERATE_MODULE_STATUSES_STR(ADXL345);

status_t adxl345_read_data(adxl345_data_t *data)
{
    status_t status = STATUS_OK;

    VALIDATE_POINTER(data, ADXL345_STATUS_INV_PTR);

    struct
    {
        int16_t x;
        int16_t y;
        int16_t z;
    } raw_data;

    status = i2c_read_target_registers(ADXL345_I2C_ADDRESS, ADXL345_DATA_X0, sizeof(raw_data), (uint8_t *)&raw_data);
    RETURN_ON_ERROR(status, status);

    data->x = 4 * (float)raw_data.x;
    data->y = 4 * (float)raw_data.y;
    data->z = 4 * (float)raw_data.z;

    return STATUS_OK;
}
