#include <netinet/in.h>  // for sockaddr_in
#include <stdint.h>      // for uint8_t
#include <stdio.h>       // for size_t
#include <sys/socket.h>  // for AF_INET, bind, recv, socket, SOCK_DGRAM
#include "common.h"      // for die, bpf_load, dump

uint8_t buffer[4096];

int main (int argc, char** argv) {
	/* Create the socket */
	struct sockaddr_in address =
	 	{ .sin_family = AF_INET
		, .sin_port   = 0x4242
		, .sin_addr   = { 0x00000000 }
		};

	int sockfd = socket (AF_INET, SOCK_DGRAM, 0);

	if (sockfd < 0) {
		die ("socket() failed");
	}

	if (bind (sockfd, (struct sockaddr*) &address, sizeof (address)) != 0) {
		die ("bind() failed");
	}

	/* Attach the filter */
	bpf_load (sockfd, "filter.c", "filter");

	/* Receive and dump any data */
	for (;;) {
		size_t size = recv (sockfd, buffer, sizeof(buffer), 0);
		dump (buffer, size);
	}
}
