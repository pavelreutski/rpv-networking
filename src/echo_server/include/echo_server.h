#pragma once

#include <stdint.h>

/**
 * @brief Starts an echo server on the specified TCP port.
 *
 * The echo server listens for incoming client connections and echoes all
 * received data back to the originating client.
 *
 * The server runs until it is explicitly stopped via
 * echoserver_shutdown().
 *
 * @param port TCP port on which to listen.
 *
 * @return true if the server was started successfully.
 * @return false if the server could not be started.
 */
bool echoserver_start(uint16_t port);

/**
 * @brief Requests shutdown of the running echo server.
 *
 * Causes the server to stop accepting new connections and terminate its
 * execution. Any resources owned by the server are released before the
 * function returns.
 *
 * @return true if the shutdown request was accepted.
 * @return false if no server is running or the shutdown request could not
 *         be processed.
 */
bool echoserver_shutdown(void);