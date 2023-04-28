/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MOCK_IREE_WRAPPER_H_
#define MOCK_IREE_WRAPPER_H_

#include <stdint.h>

enum iree_hal_element_types_t
{
    IREE_HAL_ELEMENT_TYPE_INT_8,
    IREE_HAL_ELEMENT_TYPE_UINT_8,
    IREE_HAL_ELEMENT_TYPE_INT_16,
    IREE_HAL_ELEMENT_TYPE_UINT_16,
    IREE_HAL_ELEMENT_TYPE_INT_32,
    IREE_HAL_ELEMENT_TYPE_UINT_32,
    IREE_HAL_ELEMENT_TYPE_INT_64,
    IREE_HAL_ELEMENT_TYPE_UINT_64,
    IREE_HAL_ELEMENT_TYPE_FLOAT_16,
    IREE_HAL_ELEMENT_TYPE_FLOAT_32,
    IREE_HAL_ELEMENT_TYPE_FLOAT_64,
};
typedef uint32_t iree_hal_element_type_t;

#endif // MOCK_IREE_WRAPPER_H_
