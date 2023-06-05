/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IREE_RUNTIME_UTILS_UTILS_H_
#define IREE_RUNTIME_UTILS_UTILS_H_

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
    if (status)                           \
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

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STR(STR) #STR,

#define INT_TO_BOOL(x) (!!(x))
#define LS_ONE(x) ((x) & ((x) ^ ((x) - 1)))
#define TR_ZEROS(x) (INT_TO_BOOL(LS_ONE(x) & 0xFFFF0000) << 4 |  \
                     INT_TO_BOOL(LS_ONE(x) & 0xFF00FF00) << 3 |  \
                     INT_TO_BOOL(LS_ONE(x) & 0xF0F0F0F0) << 2 |  \
                     INT_TO_BOOL(LS_ONE(x) & 0xCCCCCCCC) << 1 |  \
                     INT_TO_BOOL(LS_ONE(x) & 0xAAAAAAAA))
#define MASKED_OR_32(a, b, mask) ((a & (0xFFFFFFFF ^ mask)) | (b & mask))

#define GET_OFFSET(field) TR_ZEROS(field)
#define GET_REG_FIELD(var, field) ((var & field) >> GET_OFFSET(field))
#define SET_REG_FIELD(var, field, value) MASKED_OR_32(var, value << GET_OFFSET(field), field)

/**
 * Generic error status
 */
typedef enum {
    ERROR_STATUS_OK = 0,
    ERROR_STATUS_ERROR
} ERROR_STATUS;

#endif // IREE_RUNTIME_UTILS_UTILS_H_
