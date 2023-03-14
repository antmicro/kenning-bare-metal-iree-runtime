#ifndef IREE_RUNTIME_UTILS_PROTOCOL_H_
#define IREE_RUNTIME_UTILS_PROTOCOL_H_

#include "uart.h"

#define CHECK_UART_STATUS(status)                                                                                      \
    if (UART_TIMEOUT == status)                                                                                        \
    {                                                                                                                  \
        return SERVER_STATUS_TIMEOUT;                                                                                  \
    }                                                                                                                  \
    if (UART_OK != status)                                                                                             \
    {                                                                                                                  \
        return SERVER_STATUS_CLIENT_DISCONNECTED;                                                                      \
    }

#define MAX_MESSAGE_SIZE_BYTES (2 * 1024 * 1024) // 2MB

#define MESSAGE_SIZE_PAYLOAD(msg_size) (msg_size - sizeof(((message *)0)->message_type))
#define MESSAGE_SIZE_FULL(msg_size) (sizeof(message) + MESSAGE_SIZE_PAYLOAD(msg_size))

/**
 * An enum that describes message type
 */
#define MESSAGE_TYPES(TYPE)                                                                                            \
    TYPE(MESSAGE_TYPE_OK)                                                                                              \
    TYPE(MESSAGE_TYPE_ERROR)                                                                                           \
    TYPE(MESSAGE_TYPE_DATA)                                                                                            \
    TYPE(MESSAGE_TYPE_MODEL)                                                                                           \
    TYPE(MESSAGE_TYPE_PROCESS)                                                                                         \
    TYPE(MESSAGE_TYPE_OUTPUT)                                                                                          \
    TYPE(MESSAGE_TYPE_STATS)                                                                                           \
    TYPE(MESSAGE_TYPE_IOSPEC)                                                                                          \
    TYPE(NUM_MESSAGE_TYPES)

typedef enum
{
    MESSAGE_TYPES(GENERATE_ENUM)
} MESSAGE_TYPE;

/**
 * An enum that describes server status
 */
typedef enum
{
    SERVER_STATUS_NOTHING = 0,
    SERVER_STATUS_CLIENT_CONNECTED,
    SERVER_STATUS_CLIENT_DISCONNECTED,
    SERVER_STATUS_CLIENT_IGNORED,
    SERVER_STATUS_DATA_READY,
    SERVER_STATUS_DATA_INVALID,
    SERVER_STATUS_INTERNAL_ERROR,
    SERVER_STATUS_TIMEOUT,
} SERVER_STATUS;

/**
 * A struct that contains all parameters describing single message
 */
typedef struct __attribute__((packed))
{
    uint32_t message_size;
    uint16_t message_type;
    uint8_t payload[0];
} message;

/**
 * Waits for a message to be received
 *
 * @param msg received message
 *
 * @returns status of the server
 */
SERVER_STATUS receive_message(message **msg);
/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the server
 */
SERVER_STATUS send_message(const message *msg);
/**
 * Create a message that indicates an successful action
 *
 * @param response created message
 *
 * @returns status of the server
 */
SERVER_STATUS prepare_success_response(message **response);
/**
 * Create a message that indicates an error
 *
 * @param response created message
 *
 * @returns status of the server
 */
SERVER_STATUS prepare_failure_response(message **response);

#endif // IREE_RUNTIME_UTILS_PROTOCOL_H_