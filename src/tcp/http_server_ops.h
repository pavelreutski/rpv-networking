#pragma once

#include <stddef.h>

#include "tcp_server.h"

typedef struct {

    void    (*_free)(void *);
    void*   (*_malloc)(size_t);

    bool    (*_tcpserver_shutdown)(void);
    bool    (*_tcpserver_start)(void (*const)(tcp_conn_t const*), uint16_t);

    ssize_t (*_tcpserver_read)(tcp_conn_t const*, void *, const size_t);

} httpserver_ops_t;

void httpserver_ops(httpserver_ops_t const* ops);