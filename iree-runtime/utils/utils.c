#include "utils.h"

#define EXTERN_MODULE_STATUS_STR(module)            \
    extern const char *const module##_STATUS_STR[]; \
    extern const size_t module##_STATUS_COUNT;

MODULES(EXTERN_MODULE_STATUS_STR)

#undef EXTERN_MODULE_STATUS_STR

const char *get_status_str(status_t status)
{
    uint32_t status_code = GET_ERROR_STATUS(status);
    if (STATUS_OK == status)
    {
        return "STATUS_OK";
    }
#define CHECK_MODULE(module)                         \
    case (module):                                   \
        if (status_code < module##_STATUS_COUNT)     \
        {                                            \
            return module##_STATUS_STR[status_code]; \
        }                                            \
        return #module "_UNKNOWN_ERROR_CODE";
    switch (GET_ERROR_MODULE(status))
    {
        MODULES(CHECK_MODULE)
    default:
        return "UNKNOWN";
    }
#undef CHECK_MODULE
}
