#pragma once

#include <sys/types.h> // ssize_t
#include <stdint.h>

ssize_t huffman_decode(const uint8_t *buf, size_t buflen, uint8_t first_octet_bits, uint8_t *dest, size_t destlen);

// Decode a string that is either raw or Huffman-coded.
// The Huffman bit & length prefix are included in the buffer to decode.
// If the high bit of the first octet is set, the string is Huffman coded,
// otherwise it is raw octets.
ssize_t string_decode(const uint8_t *buf, size_t buflen, uint8_t *dest, size_t destlen);

// Same as string_decode but the output buffer is allocated using malloc
// (and does not need to be known/bounded in advance).
ssize_t string_decode_alloc(const uint8_t *buf, size_t buflen, uint8_t **destp, size_t *destlenp);
