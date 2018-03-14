#pragma once

#include <sys/types.h> // ssize_t
#include <stdint.h>

ssize_t huffman_decode(const uint8_t *buf, size_t buflen, uint8_t first_octet_bits, char *dest, size_t destlen);
