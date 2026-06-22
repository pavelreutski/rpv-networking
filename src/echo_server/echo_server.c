/** **************************************************************************
 * Includes
 * ************************************************************************** */

#include <stdio.h>

#include "tcp_server.h"
#include "echo_server.h"

#define BUFFER_SIZE                         (1024)

/** **************************************************************************
 * Private Function Declarations
 * ************************************************************************** */

static void client_handle(tcp_conn_t const* con);

/** **************************************************************************
 * Public API
 * ************************************************************************** */

bool echoserver_start(uint16_t port) {
    
    /* start server */

	printf("echo server start...\n");

	if (!tcpserver_start(client_handle, port)) {

		perror("unnable to start a server !");
		return false;
	}

    return true;
}

bool echoserver_shutdown(void) {

    printf("\necho server shutdown...\n");

	if (!tcpserver_shutdown()) {
		
		perror("unnable to stop a server !");
		return false;
	}

    return true;
}

/** **************************************************************************
 * Internal Implementation
 * ************************************************************************** */

static void client_handle(tcp_conn_t const* con) {
	
	char info[255];
	tcpserver_clientinfo(con, info, sizeof(info));

	printf("client connected %s\n", info);

	ssize_t n;
	uint8_t buffer[BUFFER_SIZE];

	while((n = tcpserver_read(con, buffer, sizeof(buffer))) > 0) {

		ssize_t m;
		printf("received %lu bytes\n", n);

		if ((m = tcpserver_write(con, buffer, n)) > 0) {
			printf("sent %lu bytes\n", m);
		}
	}
}