#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <signal.h>

#include "http_server.h"

int main(int argc, const char **argv) {

    if (argc < 2) {

        printf("port is required !\n");
        printf("usage: http_server <tcp_port>\n");

        return -1;
    }

	/* block signals the threads */

    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    uint16_t port = (uint16_t) atoi(argv[1]);

	if (!httpserver_start(port)) {
        return -1;
    }

	printf("Ctrl+C to exit...\n");

	int sgl;
	sigwait(&set, &sgl);

	if (!httpserver_shutdown()) {        
		return -1;
	}

	printf("[OK]\n");
	return 0;
}