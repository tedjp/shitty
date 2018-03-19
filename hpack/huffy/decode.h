#pragma once

#include <sys/types.h> // ssize_t
#include <stdint.h>

// TODO: Have huffman_decode decode the length prefix too.
ssize_t huffman_decode(const uint8_t *buf, size_t buflen, uint8_t first_octet_bits, uint8_t *dest, size_t destlen);
