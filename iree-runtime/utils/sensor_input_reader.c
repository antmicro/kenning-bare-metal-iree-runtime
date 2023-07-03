/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "input_reader.h"
#include "model.h"
#include "sensor.h"

GENERATE_MODULE_STATUSES_STR(INPUT_READER);

status_t read_input()
{
    status_t status = STATUS_OK;
    size_t sensor_data_size = 0;
    size_t model_input_size = 0;
    uint8_t *sensor_data = NULL;

    status = sensor_get_data_size(&sensor_data_size);
    RETURN_ON_ERROR(status, status);

    status = get_model_input_size(&model_input_size);
    RETURN_ON_ERROR(status, status);

    if (sensor_data_size * SENSOR_BUFFER_LEN != model_input_size)
    {
        return INPUT_READER_STATUS_INV_ARG;
    }

    do
    {
        status = sensor_read_data_into_buffer();
        BREAK_ON_ERROR(status);

        sensor_data = malloc(model_input_size);
        if (!IS_VALID_POINTER(sensor_data))
        {
            status = INPUT_READER_STATUS_INV_PTR;
            break;
        }

        status = sensor_get_buffered_data(model_input_size, sensor_data);
        BREAK_ON_ERROR(status);

        status = load_model_input((uint8_t *)sensor_data, model_input_size);
        BREAK_ON_ERROR(status);
    } while (0);

    if (IS_VALID_POINTER(sensor_data))
    {
        free(sensor_data);
    }

    return status;
}
