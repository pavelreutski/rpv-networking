#include "gtest/gtest.h"

extern "C" {
    
    #include "allocator.h"
    #include "tcp_server.h"
    #include "http_server.h"
}

#define HTTP_PORT                         (3434)

std::array<uint8_t, 4096> buffer;
std::array<uint8_t, 4096>::iterator buffer_it;

static std::optional<void *> _http_buffer = std::nullopt;
static std::function<void(tcp_conn_t const*)> _on_client = nullptr;

class http_server_tests : public ::testing::Test {

    protected:

        void SetUp() override {
            
            _on_client = nullptr;
            _http_buffer = nullptr;

            buffer_it = buffer.begin();
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
    ASSERT_EQ(_http_buffer.value(), buffer.data());
}

TEST_F(http_server_tests, it_should_free_io_buffer_when_http_server_stopped) {

    httpserver_start(HTTP_PORT);
    httpserver_shutdown();

    ASSERT_EQ(_http_buffer.value(), nullptr);
}

TEST_F(http_server_tests, it_should_get_connection_info_when_client_connected) {

    httpserver_start(HTTP_PORT);    
    _on_client(nullptr);

    ASSERT_STREQ(reinterpret_cast<char const*>(buffer.data()), "test client");
}

void _free(void *const ptr) {

    if (ptr == _http_buffer) {
        _http_buffer = nullptr;
    }        
}

void* _malloc(const size_t size) {

    _http_buffer = nullptr;

    if (size <= sizeof(buffer)) {

        _http_buffer = buffer.data();
        return _http_buffer.value();
    }

    return nullptr;
}

bool tcpserver_shutdown(void) {
    return _on_client != nullptr;
}

bool tcpserver_start(void (*const on_client)(tcp_conn_t const*), uint16_t port) {

    _on_client = on_client;
    return (on_client != nullptr) && (port == HTTP_PORT);
}

ssize_t tcpserver_read(tcp_conn_t const* con, void *buffer, const size_t max_len) {

    (void) con;    

    const auto available =
        static_cast<size_t>(::buffer.end() - ::buffer_it);

    const auto to_copy = std::min(max_len, available);

    ::buffer_it = std::copy_n(
        ::buffer_it,
        to_copy,
        static_cast<uint8_t*>(buffer));

    return static_cast<ssize_t>(to_copy);
}

ssize_t tcpserver_write(tcp_conn_t const *con, void const* buffer, size_t len) {

    (void) con;

    const auto remaining =
        static_cast<size_t>(::buffer.end() - ::buffer_it);

    const auto to_copy = std::min(len, remaining);

    ::buffer_it = std::copy_n(
        static_cast<const uint8_t*>(buffer),
        to_copy,
        ::buffer_it);

    return static_cast<ssize_t>(to_copy);
}

void tcpserver_clientinfo(tcp_conn_t const *con, char *client_info, size_t max_len) {    

    (void) con;

    const char client[] = "test client";

    if ((client_info == nullptr) || (max_len < sizeof(client))) {

        *client_info = '\0';
        return;
    }

    std::copy_n(client, sizeof(client), ::buffer_it);
    std::copy_n(client, sizeof(client), client_info);
}