#ifndef IREE_RUNTIME_UTILS_UTILS_H_
#define IREE_RUNTIME_UTILS_UTILS_H_

#define RETURN_ON_ERROR(status, err_code)                                                                              \
    if (status)                                                                                                        \
    {                                                                                                                  \
        return err_code;                                                                                               \
    }

#define CSR_CYCLE (0xC00)
#define CSR_TIME (0xC01)

#define CSR_READ(v, csr) __asm__ __volatile__("csrr %0, %1" : "=r"(v) : "n"(csr) : /* clobbers: none */);

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STR(STR) #STR,

#endif // IREE_RUNTIME_UTILS_UTILS_H_