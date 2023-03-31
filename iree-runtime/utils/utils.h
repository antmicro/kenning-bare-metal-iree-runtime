#ifndef IREE_RUNTIME_UTILS_UTILS_H_
#define IREE_RUNTIME_UTILS_UTILS_H_

#ifndef __UNIT_TEST__
#define ut_static static
#else // __UNIT_TEST__
#define ut_static
#endif // __UNIT_TEST__

#define IS_VALID_POINTER(ptr) (NULL != ptr)

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
        extern uint32_t mock_csr;             \
        extern void mock_csr_read_callback(); \
        v = mock_csr;                         \
        mock_csr_read_callback();             \
    } while (0);
#endif // __UNIT_TEST__

#define TIMER_CLOCK_FREQ (24000000u) /* 24 MHz */

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STR(STR) #STR,

#endif // IREE_RUNTIME_UTILS_UTILS_H_
