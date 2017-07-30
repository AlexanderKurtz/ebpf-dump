#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t

void bpf_load (int sockfd, char* file, char* function);
void die (const char* message);
void dump (uint8_t* data, size_t size);
