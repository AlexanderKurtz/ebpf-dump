#include "common.h"
#include <bcc/bpf_common.h>        // for bpf_function_size, bpf_function_start
#include <bcc/compat/linux/bpf.h>  // for bpf_prog_type::BPF_PROG_TYPE_SOCKE...
#include <bcc/libbpf.h>            // for bpf_attach_socket, bpf_prog_load
#include <stddef.h>                // for size_t, NULL
#include <stdint.h>                // for uint8_t
#include <stdio.h>                 // for printf, perror
#include <stdlib.h>                // for exit

void bpf_load (int sockfd, char* file, char* function) {
	void* module = bpf_module_create_c(file, 0, NULL, 0);
	void* start = bpf_function_start (module, function);
	size_t size = bpf_function_size (module, function);
	int progfd = bpf_prog_load (BPF_PROG_TYPE_SOCKET_FILTER, start, size, "GPL", 0, NULL, 0);

	int r = bpf_attach_socket(sockfd, progfd);

	if (r != 0) {
		die ("bpf_attach_socket() failed");
	}
}

void die (const char* message) {
	perror (message);
	exit (1);
}

void dump (uint8_t* data, size_t size) {
	printf ("Userspace: Received %zu bytes, dumping\n", size);
	for (size_t i = 0; i < size; i++) {
		printf ("\tbyte[%zu]=0x%hhx\n", i, data[i]);
	}
	printf ("\n");
}
