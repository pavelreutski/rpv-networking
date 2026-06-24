/** **************************************************************************
 * Includes
 * ************************************************************************** */

#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>

#include "tcp_server.h"
#include "http_server.h"

#define BUFFER_SIZE                                                     (4096)

/** **************************************************************************
 * Private Types
 * ************************************************************************** */

typedef struct {

    void *data;

    char *ptr;
    char *end;

    size_t size;
    size_t capacity;

} dynamic_buffer_t;

/*typedef struct {
    
} http_context_t;*/

typedef struct {
    
    dynamic_buffer_t buffer;
} httpclient_context_t;

/** **************************************************************************
 * Private State
 * ************************************************************************** */

static httpclient_context_t httpclient_ctx;

/** **************************************************************************
 * Private Function Declarations
 * ************************************************************************** */

static void client_handle(tcp_conn_t const* con);

static ssize_t httpserverclient_read(tcp_conn_t const* con, httpclient_context_t *const ctx);

static bool httpserverclient_gethttpcontext(
    httpclient_context_t *const ctx, char *restrict line, char **restrict line_memp, const size_t line_maxlen);

/** **************************************************************************
 * Public API
 * ************************************************************************** */

bool httpserver_start(uint16_t port) {    

    printf("http server start\n");

    void *http_buffer = malloc(BUFFER_SIZE);

    if (http_buffer == NULL) {
        
        printf("memory allocation for the http client buffer failed !\n");
        return false;
    }
    
    /* set client buffer init state */

    httpclient_ctx.buffer.size = BUFFER_SIZE;
    httpclient_ctx.buffer.capacity = BUFFER_SIZE;

    httpclient_ctx.buffer.data = http_buffer;

    httpclient_ctx.buffer.end = &((char *) http_buffer)[BUFFER_SIZE - 1];
    httpclient_ctx.buffer.ptr = httpclient_ctx.buffer.end;
    
    /* start tcp server */

    if (!tcpserver_start(client_handle, port)) {

        printf("unnable to start a server !\n");
        return false;
    }

    return true;
}

bool httpserver_shutdown(void) {

    printf("\nhttp server shutdown...\n");

    bool is_shutdown = tcpserver_shutdown();

    if (!is_shutdown) {
        printf("unnable to stop a server !\n");        
    }

    free(httpclient_ctx.buffer.data);

    return is_shutdown;
}

/** **************************************************************************
 * Internal Implementation
 * ************************************************************************** */

static void client_handle(tcp_conn_t const* con) {    

    char line[256];
    char *line_memp = line;

    tcpserver_clientinfo(con, line, sizeof(line));

    printf("client connected %s\n", line);

    do {

        if (httpclient_ctx.buffer.ptr == 
                httpclient_ctx.buffer.end) {

            if (httpserverclient_read(con, &httpclient_ctx) <= 0) {
                return;
            }
        }

    } while (!httpserverclient_gethttpcontext(&httpclient_ctx, line, &line_memp, sizeof(line) - 1));

    int n = 0;
    while(httpserverclient_read(con, &httpclient_ctx) > 0) { n++; }
}

static ssize_t httpserverclient_read(tcp_conn_t const* con, httpclient_context_t *const ctx) {

    void *client_data = ctx -> buffer.data;
    size_t capacity = ctx -> buffer.capacity;

    ssize_t read_size = tcpserver_read(con, client_data, capacity);

    if (read_size <= 0) {
        return read_size;
    }

    ctx -> buffer.size = read_size;
    ctx -> buffer.ptr = client_data;

    ctx -> buffer.end = &(ctx -> buffer.ptr)[read_size];

    return read_size;
}

static bool httpserverclient_gethttpcontext(
    httpclient_context_t *const ctx, char *restrict line, char **restrict line_memp, const size_t line_maxlen) {    

    size_t n = 0, m = 0;
    char *s = *line_memp;

    uint16_t line_reg = 0;
    uint8_t context_reg = 0;

    size_t max_len = line_maxlen - (size_t)(s - line) - 1;

    while((*line != '\0') && (n < max_len) && 
            ((ctx -> buffer.ptr) != (ctx -> buffer.end))) {

        char c = *(ctx -> buffer.ptr);
        
        if ((c != '\r') && (c != '\n')) {

            n++;
            *s++ = c;
        }

        line_reg <<= 8;
        line_reg |= c;
        
        bool is_ln = (line_reg == 0x0d0a);

        switch(context_reg) {

            default: {

                if (is_ln) {

                    *s = '\0';
                    s = line;

                    m++;
                    printf("line#%lu %s\n", m, line);
                }
            }
        }
        
        (ctx -> buffer.ptr)++;
    }

    *line_memp = s;

    return (*line == '\0');
}