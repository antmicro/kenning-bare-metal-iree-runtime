/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_UTILS_H_
#define IREE_RUNTIME_UTILS_UTILS_H_

#include <stddef.h>
#include <stdint.h>

#ifndef __UNIT_TEST__
#define ut_static static
#else // __UNIT_TEST__
#define ut_static
#endif // __UNIT_TEST__

#define IS_VALID_POINTER(ptr) (NULL != (ptr))

#define VALIDATE_POINTER(ptr, error_status) \
    if (!IS_VALID_POINTER(ptr))             \
    {                                       \
        return error_status;                \
    }

#define RETURN_ON_ERROR(status, err_code) \
    if (STATUS_OK != (status))            \
    {                                     \
        return err_code;                  \
    }

#define CSR_CYCLE (0xC00)
#define CSR_TIME (0xC01)

#ifndef __UNIT_TEST__
#define CSR_READ(v, csr) __asm__ __volatile__("csrr %0, %1" : "=r"(v) : "n"(csr) : /* clobbers: none */);
#else // __UNIT_TEST__
#define CSR_READ(v, csr)                      \
    do                                        \
    {                                         \
        extern uint32_t g_mock_csr;           \
        extern void mock_csr_read_callback(); \
        (v) = g_mock_csr;                     \
        mock_csr_read_callback();             \
    } while (0);
#endif // __UNIT_TEST__

#define TIMER_CLOCK_FREQ (24000000u) /* 24 MHz */

#define GENERATE_ENUM(enum, ...) enum,
#define GENERATE_STR(str, ...) #str,

#define INT_TO_BOOL(x) (!!(x))
/* extracts least significant one */
#define LS_ONE(x) ((x) & ((x) ^ ((x)-1)))
/* counts tailing zeros */
#define TR_ZEROS(x)                                                                        \
    (INT_TO_BOOL(LS_ONE(x) & 0xFFFF0000) << 4 | INT_TO_BOOL(LS_ONE(x) & 0xFF00FF00) << 3 | \
     INT_TO_BOOL(LS_ONE(x) & 0xF0F0F0F0) << 2 | INT_TO_BOOL(LS_ONE(x) & 0xCCCCCCCC) << 1 | \
     INT_TO_BOOL(LS_ONE(x) & 0xAAAAAAAA))
/* performs masked or operation */
#define MASKED_OR_32(a, b, mask) (((a) & (0xFFFFFFFF ^ (mask))) | ((b) & (mask)))

/* retrieves offset from register field mask */
#define GET_OFFSET(field) TR_ZEROS(field)
/* extracts register field value */
#define GET_REG_FIELD(var, field) (((var) & (field)) >> GET_OFFSET(field))
/* sets register field value */
#define SET_REG_FIELD(var, field, value) (MASKED_OR_32((var), (value) << GET_OFFSET(field), (field)))

#define ERROR_MASK_MODULE 0xFF00
#define ERROR_MASK_STATUS 0xFF
#define GENERATE_ERROR(module, status)                                 \
    ((((module) << TR_ZEROS(ERROR_MASK_MODULE)) & ERROR_MASK_MODULE) | \
     ((((status_t)(status)) << TR_ZEROS(ERROR_MASK_STATUS)) & ERROR_MASK_STATUS))
#define GET_ERROR_MODULE(status) GET_REG_FIELD(status, ERROR_MASK_MODULE)
#define GET_ERROR_STATUS(status) GET_REG_FIELD(status, ERROR_MASK_STATUS)
#define GENERATE_MODULE_STATUSES(module)                                                              \
    typedef enum                                                                                      \
    {                                                                                                 \
        module##_STATUS_OK = GENERATE_ERROR(module, STATUS_OK),                                       \
        GENERIC_STATUSES(GENERATE_ENUM, module) module##_STATUSES(GENERATE_ENUM) module##_LAST_STATUS \
    } module##_STATUS;

#define GENERATE_MODULE_STATUSES_STR(module)                                                                \
    const char *const module##_STATUS_STR[] = {#module "_STATUS_OK", GENERIC_STATUSES(GENERATE_STR, module) \
                                                                         module##_STATUSES(GENERATE_STR)};  \
    const size_t module##_STATUS_COUNT = GET_STATUS_COUNT(module);

#define GET_STATUS_COUNT(module) GET_ERROR_STATUS(module##_LAST_STATUS)

/**
 * Modules
 */
#define MODULES(MODULE)  \
    MODULE(RUNTIME)      \
    MODULE(MODEL)        \
    MODULE(IREE_WRAPPER) \
    MODULE(PROTOCOL)     \
    MODULE(UART)         \
    MODULE(I2C)          \
    MODULE(ADXL345)      \
    MODULE(SENSOR)

enum
{
    SKIP_ZERO,
    MODULES(GENERATE_ENUM)
};

#define STATUS_OK 0 /* success */

/**
 * Generic error status
 */
#define GENERIC_STATUSES(STATUS, module)                       \
    STATUS(module##_STATUS_ERROR)   /* generic error */        \
    STATUS(module##_STATUS_INV_PTR) /* invalid pointer */      \
    STATUS(module##_STATUS_INV_ARG) /* invalid argument */     \
    STATUS(module##_STATUS_UNINIT)  /* module uninitialized */ \
    STATUS(module##_STATUS_TIMEOUT) /* timeout */

typedef uint32_t status_t;

const char *get_status_str(status_t status);

#endif // IREE_RUNTIME_UTILS_UTILS_H_
