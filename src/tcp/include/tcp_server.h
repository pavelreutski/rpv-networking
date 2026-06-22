#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Opaque TCP client connection handle.
 *
 * The connection handle is owned by the TCP server and is only valid
 * during execution of the client callback.
 *
 * The connection is automatically closed when the callback returns.
 *
 * The handle must not be stored, copied, or used outside the callback.
 */
typedef struct tcp_conn tcp_conn_t;

/**
 * @brief Starts a TCP server on the specified port.
 *
 * The server listens for incoming client connections. For each accepted client,
 * the supplied callback is invoked with a connection handle that can be used to
 * communicate with the client.
 *
 * The callback is invoked from an internal server listening thread
 * 
 * @warning The connection handle supplied to the callback is valid only for 
 * the duration of the callback invocation. Applications must not retain, 
 * copy, or access the handle after the callback returns.
 *
 * @param on_client Callback invoked when a client connection is accepted.
 * @param port TCP port on which to listen.
 *
 * @return true if the server was started successfully.
 * @return false if the server could not be started.
 */
bool tcpserver_start(
    void (*const on_client)(tcp_conn_t const *),
    uint16_t port
);

/**
 * @brief Requests shutdown of the running TCP server.
 *
 * Causes the server accept loop to terminate and releases any resources owned
 * by the server.
 *
 * @return true if the shutdown request was accepted.
 * @return false if no server is running or the request could not be processed.
 */
bool tcpserver_shutdown(void);

/**
 * @brief Reads data from a client connection.
 *
 * Attempts to read up to @p max_len bytes from the specified connection into
 * the supplied buffer. 
 * 
 * @warning The connection handle and this function must have been supplied from the currently executing 
 * client callback and must not be used after the callback returns.
 *
 * @param con Client connection handle.
 * @param buffer Destination buffer.
 * @param max_len Maximum number of bytes to read.
 *
 * @return Number of bytes read.
 * @retval 0 The peer has closed the connection.
 * @retval -1 An error occurred.
 */
ssize_t tcpserver_read(
    tcp_conn_t const *con,
    void *buffer,
    size_t max_len
);

/**
 * @brief Writes data to a client connection.
 *
 * Attempts to send up to @p len bytes from the supplied buffer to the client.
 * 
 * @warning The connection handle and this function must have been supplied from the currently executing 
 * client callback and must not be used after the callback returns.
 *
 * @param con Client connection handle.
 * @param buffer Source buffer containing data to send.
 * @param len Number of bytes to write.
 *
 * @return Number of bytes written.
 * @retval -1 An error occurred.
 */
ssize_t tcpserver_write(
    tcp_conn_t const *con,
    void const *buffer,
    size_t len
);

/**
 * @brief Retrieves information about the remote client.
 *
 * Writes a null-terminated textual representation of the remote endpoint
 * (for example, an IP address and port) into the supplied buffer.
 *
 * If the resulting string exceeds @p max_len, it is truncated.
 * 
 * @warning The connection handle and this function must have been supplied from the currently executing 
 * client callback and must not be used after the callback returns.
 *
 * @param con Client connection handle.
 * @param client_info Output buffer receiving the client information string.
 * @param max_len Size of @p client_info in bytes.
 */
void tcpserver_clientinfo(
    tcp_conn_t const *con,
    char *client_info,
    size_t max_len
);
