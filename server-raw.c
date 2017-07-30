#include <stdint.h>      // for uint8_t
#include <stdio.h>       // for size_t
#include <sys/socket.h>  // for recv, socket, AF_INET, SOCK_RAW
#include "common.h"      // for bpf_load

uint8_t buffer[4096];

int main (int argc, char** argv) {
	/* Create the socket */
	int sockfd = socket (AF_INET, SOCK_RAW, 6 /* TCP */);

	/* Attach the filter */
	bpf_load (sockfd, "filter.c", "filter");

	/* Receive and dump any data */
	for (;;) {
		size_t size = recv (sockfd, buffer, sizeof(buffer), 0);
		dump (buffer, size);
	}
}
