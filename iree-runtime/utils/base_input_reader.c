/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "input_reader.h"

GENERATE_MODULE_STATUSES_STR(INPUT_READER);

status_t read_input() { return INPUT_READER_NO_READ; }
