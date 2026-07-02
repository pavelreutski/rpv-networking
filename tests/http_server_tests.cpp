#include "gtest/gtest.h"

extern "C" {
    
    #include "allocator.h"
    #include "tcp_server.h"
    #include "http_server.h"
}

#define HTTP_PORT                         (3434)

static uint8_t buffer[4096];

static void *_http_buffer = nullptr;
static void (*_on_client)(tcp_conn_t const*) = nullptr;

class http_server_tests : public ::testing::Test {

    protected:

        void SetUp() override {
            
            _on_client = nullptr;
            _http_buffer = nullptr;
        }

        void TearDown() override {
        }
};

TEST_F(http_server_tests, it_should_start_http_server) {        
    ASSERT_TRUE(httpserver_start(HTTP_PORT));
}

TEST_F(http_server_tests, it_should_stop_http_server) {

    httpserver_start(HTTP_PORT);
    ASSERT_TRUE(httpserver_shutdown());   
}

TEST_F(http_server_tests, it_should_allocate_io_buffer_when_http_server_started) {

    httpserver_start(HTTP_PORT);
    ASSERT_EQ(_http_buffer, buffer);
}

TEST_F(http_server_tests, it_should_free_io_buffer_when_http_server_stopped) {

    httpserver_start(HTTP_PORT);
    httpserver_shutdown();

    ASSERT_EQ(_http_buffer, nullptr);
}

void _free(void *const ptr) {

    if (ptr == _http_buffer) {
        _http_buffer = nullptr;
    }        
}

void* _malloc(const size_t size) {

    _http_buffer = nullptr;

    if (size <= sizeof(buffer)) {

        _http_buffer = buffer;
        return _http_buffer;
    }

    return nullptr;
}

ssize_t tcpserver_read(tcp_conn_t const* con, void *buffer, const size_t max_len) {

    (void) con;
    (void) buffer;
    (void) max_len;

    return -1;
}

bool tcpserver_shutdown(void) {
    return _on_client != nullptr;
}

bool tcpserver_start(void (*const on_client)(tcp_conn_t const*), uint16_t port) {

    _on_client = on_client;
    return (on_client != nullptr) && (port == HTTP_PORT);
}

ssize_t tcpserver_write(tcp_conn_t const *con, void const *buffer, size_t len) {

    (void) con;
    (void) buffer;
    (void) len;

    return -1;
}

void tcpserver_clientinfo(tcp_conn_t const *con, char *client_info, size_t max_len) {

    (void) con;
    (void) client_info;
    (void) max_len;
}