/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_ADXL345_H_
#define IREE_RUNTIME_UTILS_ADXL345_H_

#include "springbok.h"

#include "i2c.h"

#define ADXL345_ADDRESS (0x80)
#define ADXL345_DATA_X0 (0x32)

#define ADXL345_DATA_FORMAT (0x31)
#define ADXL345_DATA_FORMAT_RANGE_BITS (0x3 << 0)

ERROR_STATUS adxl345_read_data(int16_t *x, int16_t *y, int16_t *z);

#endif // IREE_RUNTIME_UTILS_ADXL345_H_
