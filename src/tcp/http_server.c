/** **************************************************************************
 * Includes
 * ************************************************************************** */

#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "tcp_server.h"
#include "http_server.h"

#define BUFFER_SIZE                                                     (4096)

/** **************************************************************************
 * Private Types
 * ************************************************************************** */

typedef enum {

    http_ok,
    http_ioerror,
    http_badrequest,
    http_notsupported,    
} httpcontext_result_t;

typedef struct {

    void *data;

    char *ptr;
    char *end;

    size_t size;
    size_t capacity;

} dynamic_buffer_t;

typedef struct {

    char name[64];
    char value[256]; 
} httprequest_header_t;

typedef struct {

    char method[16];
    char uri[2048];
    char version[16];

    httprequest_header_t header;
    
} httprequest_context_t;

/*typedef struct {

} httpresponse_context_t;*/

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

static httpcontext_result_t httpserverclient_gethttpcontext(
    tcp_conn_t const* con, httpclient_context_t *const client_ctx, httprequest_context_t *const request_ctx);

/** **************************************************************************
 * Public API
 * ************************************************************************** */

bool httpserver_start(uint16_t port) {    

    printf("http server start\n");

    void *http_buffer = _malloc(BUFFER_SIZE);

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

    _free(httpclient_ctx.buffer.data);

    return is_shutdown;
}

/** **************************************************************************
 * Internal Implementation
 * ************************************************************************** */

static void client_handle(tcp_conn_t const* con) {    

    char client_info[256];    
    tcpserver_clientinfo(con, client_info, sizeof(client_info));

    printf("client connected %s\n", client_info);    

    httprequest_context_t request_ctx;
    httpcontext_result_t httpresult = httpserverclient_gethttpcontext(con, &httpclient_ctx, &request_ctx);

    if (httpresult == http_ok) {
        printf("method: '%s' uri: '%s' version: '%s'\n", request_ctx.method, request_ctx.uri, request_ctx.version);
    }

    if (httpresult == http_notsupported) {
        printf("request not supported");
    }

    while (httpserverclient_read(con, &httpclient_ctx) > 0);
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

static httpcontext_result_t httpserverclient_gethttpcontext(
    tcp_conn_t const* con, httpclient_context_t *const client_ctx, httprequest_context_t *const request_ctx) {

    enum : uint8_t {

        http_method,
        http_resource,
        http_version,
        http_headername,
        http_headervalue,
        http_default

    } context_reg = http_method; 
    
    memset((void *) request_ctx, 0xCC, sizeof(httprequest_context_t));

    size_t param_len = 0;

    char *param = request_ctx -> method;
    size_t parmax_len = sizeof(request_ctx -> method);

    uint16_t line_reg = 0;

    while((*param != '\0') && (param_len < parmax_len)) {

        if (client_ctx -> buffer.ptr == 
                client_ctx -> buffer.end) {

            if (httpserverclient_read(con, client_ctx) <= 0) {
                return http_ioerror;
            }
        }

        char c = *(client_ctx-> buffer.ptr);
        
        if ((c != '\r') && (c != '\n')) {
            
            *param++ = c;
            param_len++;
        }

        line_reg <<= 8;
        line_reg |= c;
        
        bool isln = (line_reg == 0x0d0a);

        switch(context_reg) {

            case http_method: {

                if (isspace(c)) {
                                        
                    *(param - 1) = '\0';
                    
                    param = request_ctx -> uri;
                    parmax_len = sizeof(request_ctx -> uri);

                    param_len = 0;
                    context_reg = http_resource;
                }

            } break;

            case http_resource: {

                if (isspace(c)) {
                    
                    *(param - 1) = '\0';                    

                    param = request_ctx -> version;
                    parmax_len = sizeof(request_ctx -> version);

                    param_len = 0;
                    context_reg = http_version;
                }

            } break;

            case http_version: {

                if (isln) {

                    *param = '\0';

                    param = request_ctx -> header.name;
                    parmax_len = sizeof(request_ctx -> header.name);

                    param_len = 0;
                    context_reg = http_headername;
                }

            } break;

            case http_headername: {

                if (isln) {

                    *param = '\0';
                    param_len = 0;
                }

                if (c == ':') {

                    *(param - 1) = '\0';

                    param = request_ctx -> header.value;
                    parmax_len = sizeof(request_ctx -> header.value);

                    param_len = 0;
                    context_reg = http_headervalue;                    
                }                

            } break;

            case http_headervalue: {

                if (isln) {

                    *param = '\0';

                    printf("header: %s value: %s\n", 
                        request_ctx -> header.name, request_ctx -> header.value);

                    memset(request_ctx -> header.name, 0xCC, sizeof(request_ctx -> header.name));
                    memset(request_ctx -> header.value, 0xCC, sizeof(request_ctx -> header.value));
                    
                    param = request_ctx -> header.name;
                    parmax_len = sizeof(request_ctx -> header.name);
                    
                    param_len = 0;
                    context_reg = http_headername;
                }

            } break;

            default: break;
        }
        
        (client_ctx -> buffer.ptr)++;
    }

    return (*param == '\0') ? http_ok : http_notsupported;
}