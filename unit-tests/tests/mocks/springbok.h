/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#define LOG_ERROR(msg, args...) printf("[ERROR] " msg, ##args);
#define LOG_WARN(msg, args...) printf("[WARN] " msg, ##args);
#define LOG_INFO(msg, args...) printf("[INFO] " msg, ##args);
#define LOG_DEBUG(msg, args...) printf("[DEBUG] " msg, ##args);
#define LOG_NOISY(msg, args...) printf("[NOISY] " msg, ##args);