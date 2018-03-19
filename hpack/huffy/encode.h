#ifndef HUFFY_ENCODE_H
#define HUFFY_ENCODE_H

#include <stdint.h>
#include <sys/types.h>

ssize_t huffman_encode(const uint8_t *input, size_t input_len, uint8_t *buf, size_t buflen);

#endif
