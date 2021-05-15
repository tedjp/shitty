#include <stddef.h>
#include <stdint.h>
#include <sys/types.h> // ssize_t

#ifdef __cplusplus
extern "C" {
#endif

// bits: how many bits to use from the first field. Often 7 in HPACK.
// returns how many octets were used including the first (partial) octet,
// or negative error code.
ssize_t decode_number(const uint8_t* buf, size_t len, uint_fast8_t bits, uintmax_t *number);

// Return number of bytes used for encoding (including the first partial byte).
// Returns negative value on error.
ssize_t encode_number(uintmax_t number, uint_fast8_t prefix_bits, uint8_t *buf, size_t buflen);

#ifdef __cplusplus
} // extern "C"
#endif
