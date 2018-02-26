#ifndef HUFFY_ENCODE_H
#define HUFFY_ENCODE_H

#include <stdint.h>
#include <sys/types.h>

ssize_t huffman_encode(const char *str, uint8_t *buf, size_t buflen);

#endif
