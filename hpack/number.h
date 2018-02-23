#include <stddef.h>
#include <stdint.h>
#include <sys/types.h> // ssize_t

// bits: how many bits to use from the first field. Often 7 in HPACK.
int decode_number(const uint8_t* buf, size_t len, uint_fast8_t bits, uintmax_t *number);

// Return number of *extra* bytes used for encoding (not including the bits in
// the prefix/first byte). Returns zero for all values that fit in the prefix.
// Returns negative value on error.
ssize_t encode_number(uintmax_t number, uint_fast8_t prefix_bits, uint8_t *buf, size_t buflen);
