/** **************************************************************************
 * Includes
 * ************************************************************************** */

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
static bool httpserverclient_getline(httpclient_context_t *const ctx, char *const line, size_t *const len, const size_t max_len);

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

    printf("\ntcp server shutdown...\n");

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

    int n = 0;

    char line[256];
    char *s = line;

    do {

        if (httpclient_ctx.buffer.ptr == 
                httpclient_ctx.buffer.end) {

            if (httpserverclient_read(con, &httpclient_ctx) <= 0) {
                return;
            }
        }
        
        size_t len;
        size_t max_len = (sizeof(line) - (size_t)(s - line)) - 1;

        if(httpserverclient_getline(&httpclient_ctx, s, &len, max_len)) {

            n++;
            s = line;

            printf("line#%d %s\n", n, line);

        } else {
            s += len;
        }        

    } while (*line != '\0');

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

static bool httpserverclient_getline(httpclient_context_t *const ctx, char *const line, size_t *const len, const size_t max_len) {    

    size_t n = 0;

    char *s = line;
    uint16_t line_reg = 0;

    while((line_reg != 0x0d0a) && (n < max_len) && 
            ((ctx -> buffer.ptr) != (ctx -> buffer.end))) {

        char c = *(ctx -> buffer.ptr);
        
        if ((c != '\r') && (c != '\n')) {

            n++;
            *s++ = c;
        }

        line_reg <<= 8;
        line_reg |= c;

        (ctx -> buffer.ptr)++;
    }

    *len = n;

    bool is_line = (line_reg == 0x0d0a);

    if (is_line) {
        *s = '\0';
    }

    return (line_reg == 0x0d0a);
}