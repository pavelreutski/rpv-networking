#include "gtest/gtest.h"

extern "C" {
        
    #include "http_server.h"
    #include "http_server_ops.h"
}

#define HTTP_PORT                         (3434)

static uint8_t buffer[4096];

static void (*_on_client)(tcp_conn_t const*) = nullptr;

static void mock_free(void *ptr) {
    (void) ptr;
}

static void* mock_malloc(size_t size) {
    return size <= sizeof(buffer) ? buffer : nullptr;
}

ssize_t mock_tcpserver_read(tcp_conn_t const* con, void *buffer, const size_t max_len) {

    (void) con;
    (void) buffer;
    (void) max_len;

    return -1;
}

static bool mock_tcpserver_shutdown(void) {
    return _on_client != nullptr;
}

static bool mock_tcpserver_start(void (*const on_client)(tcp_conn_t const*), uint16_t port) {

    _on_client = on_client;
    return (on_client != nullptr) && (port == HTTP_PORT);
}

class http_server_tests : public ::testing::Test {

    protected:

        void SetUp() override {

            _on_client = nullptr;

            httpserver_ops_t ops = {

                _free: mock_free,
                _malloc: mock_malloc,

                _tcpserver_shutdown: mock_tcpserver_shutdown,
                _tcpserver_start: mock_tcpserver_start,
                _tcpserver_read : mock_tcpserver_read };

            httpserver_ops(&ops);
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