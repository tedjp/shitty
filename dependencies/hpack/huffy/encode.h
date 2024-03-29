#ifndef HUFFY_ENCODE_H
#define HUFFY_ENCODE_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns negative value on error.
// On error, the caller should probably just use a non-encoded string.
// This does *not* fail if the encoded version is longer than the plaintext.
// You should check the length yourself and decide whether to use the encoded
// version.
// (However if you size your output buffer the same length as the input,
// it'll fail if the encoded version is longer.)
// Encoded version includes the length prefix, so you might want to provide room
// for it in your buffer ;)
ssize_t huffman_encode(const uint8_t *input, size_t input_len, uint8_t *buf, size_t buflen);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
