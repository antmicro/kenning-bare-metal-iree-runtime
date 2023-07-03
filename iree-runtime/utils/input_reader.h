/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTIL_INPUT_READER_H_
#define IREE_RUNTIME_UTIL_INPUT_READER_H_

#include "utils.h"

/**
 * Input reader custom error codes
 */
#define INPUT_READER_STATUSES(STATUS) STATUS(INPUT_READER_NO_READ)

GENERATE_MODULE_STATUSES(INPUT_READER);

/**
 * Reads data and loads it into model
 *
 * @returns error status
 */
status_t read_input();

#endif // IREE_RUNTIME_UTIL_INPUT_READER_H_
