/** **************************************************************************
 * Includes
 * ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <poll.h>
#include <errno.h>

#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "tcp_server.h"

/** **************************************************************************
 * Private Types
 * ************************************************************************** */

typedef union {

    int fds[2];

    struct {

        int r_fd;
        int w_fd;
    };

} server_pipe_t;

typedef union {

    struct pollfd fds[2];

    struct {

        struct pollfd pipe_poll;
        struct pollfd server_poll;        
    };

} server_iocontext_t;

typedef struct {

    int fd;
    struct sockaddr_in addr;

    void (*const con_callback)(tcp_conn_t const* con);
} tcp_server_t;

struct tcp_conn {

    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
};

/** **************************************************************************
 * Private State
 * ************************************************************************** */

static server_pipe_t server_pipe;

static pthread_t server_tid = ULONG_MAX;

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

/** **************************************************************************
 * Private Function Declarations
 * ************************************************************************** */

static void* tcpserver_runnable(void *arg);

static void tcpserver_close(tcp_server_t *server);
static bool tcpserver_create(tcp_server_t *server);

/** **************************************************************************
 * Public API
 * ************************************************************************** */

ssize_t tcpserver_read(tcp_conn_t const* con, void *buffer, const size_t max_len) {
    return (con != NULL) ? read(con -> client_fd, buffer, max_len) : -1;
}

ssize_t tcpserver_write(tcp_conn_t const* con, void const* buffer, const size_t len) {

    if ((con == NULL) || (len > SSIZE_MAX)) { return -1; }
    	
	ssize_t max_len = len;
	ssize_t n = 0, m = len;

    uint8_t *buff = (uint8_t *) buffer;

    while ((n < max_len) && (m > 0)) {

        m = write(con -> client_fd, buff, (max_len - n));

        if (m > 0) {
        
            n += m;
            buff += m;
        }
    }

    return (m < 0) ? (n > 0 ? n : m) : n;
}

void tcpserver_clientinfo(tcp_conn_t const* con, char *client_info, const size_t max_len) {

    if ((con == NULL) || (client_info == NULL)) { return; }

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, (void *) &con -> client_addr.sin_addr, ip, sizeof(ip));

	snprintf(client_info, max_len, "%s:%u", ip, ntohs(con -> client_addr.sin_port));
}

bool tcpserver_shutdown(void) {

    if (server_tid == ULONG_MAX) {

         perror("server is not alive !");
         return false;
    }

	pthread_mutex_lock(&mux);

    const bool stop_flag = true;
    if (write(server_pipe.w_fd, &stop_flag, sizeof(stop_flag)) < 0) {

        perror("server pipe is broken !!!");

		pthread_mutex_unlock(&mux);
		return false;
    }

	pthread_cond_wait(&cond, &mux);
	pthread_mutex_unlock(&mux);

	pthread_join(server_tid, NULL);

    server_tid = ULONG_MAX;

	return true;
}

bool tcpserver_start(void (*const on_client)(tcp_conn_t const*), const uint16_t port) {    

    tcp_server_t server = { 
        .con_callback = on_client };
    
	server.addr.sin_family = AF_INET;
	server.addr.sin_port = htons(port);
	server.addr.sin_addr.s_addr = htonl(INADDR_ANY);	

	if (!tcpserver_create(&server)) {

		tcpserver_close(&server);
		return false;
	}

	pthread_mutex_lock(&mux);
    
	pthread_create(&server_tid, NULL, tcpserver_runnable, (void *) &server);

	pthread_cond_wait(&cond, &mux);
	pthread_mutex_unlock(&mux);

	return true;
}

/** **************************************************************************
 * Internal Implementation
 * ************************************************************************** */

static void tcpserver_close(tcp_server_t *server) {	

    int server_fd = server -> fd;

    int pipe_wfd = server_pipe.w_fd;
    int pipe_rfd = server_pipe.r_fd;

	if (server_fd > 0) {

		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
	}

	if (pipe_rfd > 0) {

		close(pipe_rfd);
		close(pipe_wfd);
	}

    server_pipe.r_fd = -1;
    server_pipe.w_fd = -1;

    memset((void *) server, -1, sizeof(tcp_server_t));
}

static bool tcpserver_create(tcp_server_t *server) {
	
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_fd < 0) {

		perror("server socket creation failed !");
		return false;
	}

	int reuseAddr_opt = true;
	if (setsockopt(sock_fd, 
			SOL_SOCKET, SO_REUSEADDR, &reuseAddr_opt, sizeof(reuseAddr_opt)) < 0) {

		perror("reuse address option set for server secoket failed !");
		
		close(sock_fd);
		return false;
	}

    socklen_t addr_len = sizeof(server -> addr);
	if (bind(sock_fd, (struct sockaddr *) &(server -> addr), addr_len) < 0) {

		perror("bind server socket to ANY address failed !");

		close(sock_fd);
		return false;
	}

	if (listen(sock_fd, 16) < 0) {

		perror("listen on server socket failed !");		

		close(sock_fd);
		return false;
	}

	/* create server pipe */

    if (pipe((int *) (server_pipe.fds)) < 0) {

        perror("server pipe create failed !!!");

		close(sock_fd);
        return false;
    }

	server -> fd = sock_fd;

    return true;
}

static void* tcpserver_runnable(void *arg) {	

    volatile tcp_server_t server;

	pthread_mutex_lock(&mux);

    memcpy((void *) &server, arg, sizeof(tcp_server_t));
	printf("tcp server listening on port %hu\n", ntohs(server.addr.sin_port));

	/* notify server started */
	
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mux);

	/* server loop */

	volatile server_iocontext_t io;
	memset((void *) &io, 0, sizeof(io));

	io.server_poll.fd = server.fd;
    io.server_poll.events = POLLIN;

    io.pipe_poll.events = POLLIN;
    io.pipe_poll.fd = server_pipe.r_fd;

	while(true) {
		
        io.pipe_poll.revents = 0;
		io.server_poll.revents = 0;
        
        if (poll((struct pollfd *) io.fds, 2, -1) < 0) {

			perror("broken server io context !");
			continue;
		}

        if (io.pipe_poll.revents & POLLIN) {
            break;
        }

		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(addr);

        int client_fd = accept(server.fd, (struct sockaddr *) &addr, &addr_len);

		if (client_fd < 0) {

			perror("accept failed !");
			continue;
		}

		volatile struct tcp_conn con;

        con.client_fd = client_fd;
        
        memcpy((void *) &con.client_addr, (void *) &addr, addr_len);
        memcpy((void *) &con.server_addr, (void *) &server.addr, addr_len);

        server.con_callback((tcp_conn_t *) &con);
		
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);

		printf("tcp server client socket closed\n");
	}

	pthread_mutex_lock(&mux);

	tcpserver_close((tcp_server_t *) &server);
	printf("tcp server socket closed\n");

	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mux);

	return NULL;
}