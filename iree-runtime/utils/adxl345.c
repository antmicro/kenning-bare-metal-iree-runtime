/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "adxl345.h"

ERROR_STATUS adxl345_read_data(int16_t *x, int16_t *y, int16_t *z)
{
    I2C_STATUS i2c_status = I2C_STATUS_OK;
    uint8_t data[6];

    i2c_status = i2c_read_target_registers(ADXL345_ADDRESS, ADXL345_DATA_X0, 6, data);
    RETURN_ON_ERROR(i2c_status, ERROR_STATUS_ERROR);

    *x = data[0] | (data[1] << 8);
    *y = data[2] | (data[3] << 8);
    *z = data[4] | (data[5] << 8);

    return ERROR_STATUS_OK;
}
