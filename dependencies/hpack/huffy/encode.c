#include <limits.h>
#include <string.h>

#include "encode.h"
#include "encode-table.h"
#include "../number.h"

// Once the size is known it can be prefixed with the H[uffman] bit and the
// 7-bit prefixed length prefix.
// The string is always left-aligned using the full byte.
static ssize_t encode(const uint8_t *input, size_t input_len, uint8_t *buf, size_t buflen) {
    // must always be [0-7]
    uint_fast8_t existing_bits = 0;
    uint8_t partial = 0; // partial octet
    size_t outi = 0;

    for (size_t i = 0; i < input_len; ++i) {
        const struct encv encv = etable[input[i]];
        uint32_t huffcode = encv.value;
        uint_fast8_t bits = encv.bits;

        if (existing_bits) {
            if (bits + existing_bits >= 8) {
                partial |= (uint8_t)(huffcode >> (bits - (8 - existing_bits)));

                bits -= (8 - existing_bits);
                huffcode &= (1 << bits) - 1;

                if (__builtin_expect(outi >= buflen, 0))
                    return -1;

                buf[outi] = partial;
                ++outi;

                existing_bits = 0;
                partial = 0;
            } else {
                uint_fast8_t new_partial_bits = 8 - existing_bits;
                if (new_partial_bits > bits)
                    new_partial_bits = bits;
                partial |= (uint8_t)(huffcode << (8 - (existing_bits + bits)));
                existing_bits += new_partial_bits;
                bits -= new_partial_bits;
            }
        }

        while (bits > 8) {
            // write full octet
            if (__builtin_expect(outi >= buflen, 0))
                return -1;

            buf[outi] = (uint8_t)(huffcode >> (bits - 8));
            ++outi;

            bits -= 8;
        }

        if (bits) {
            existing_bits = bits;
            partial = huffcode << (8 - bits);
        }
    }

    // Write out the remaining partial octet, padding the right-hand-side
    // with 1s (EOS).
    if (existing_bits) {
        if (__builtin_expect(outi >= buflen, 0))
            return -1;

        partial |= (1 << (8 - existing_bits)) - 1;
        buf[outi] = partial;
        ++outi;
    }

    if (__builtin_expect(outi > SSIZE_MAX, 0)) {
        return -1;
    }

    return (ssize_t)outi;
}

ssize_t huffman_encode(const uint8_t *input, size_t input_len, uint8_t *buf, size_t buflen) {
    if (buflen < 1) {
        // No room for length + data
        return -1;
    }

    // Encode directly into the caller's buffer.
    //
    // Hopes that the encoded length will be <128, if not the encoded string
    // will be shifted forward in the buffer by whatever the length of the
    // number prefix is.
    ssize_t len = encode(input, input_len, buf + 1, buflen - 1);

    if (len < 0)
        return len;

    size_t stlen = (size_t) len;

    // If the encoded length is >= 127, the encoded string has to be moved
    // right by as many bytes as the encoded number will take up.

    // Encode the number and determine how long it really is.
    // Largest encoded 64-bit value is 10 octets which we determine to be the
    // biggest number we care to support.
    uint8_t encoded_number[10];
    ssize_t nlen = encode_number(stlen, 7, encoded_number,
            sizeof(encoded_number));
    if (nlen < 0)
        return nlen;

    // Check for integer overflow before adding both lengths
    if (__builtin_expect(SSIZE_MAX - len < nlen, 0)) {
        return -1;
    }

    if (nlen > 1) {
        memmove(buf + nlen, buf + 1, stlen);
    }

    // Copy the encoded number in
    memcpy(buf, encoded_number, nlen);

    // Set Huffman bit
    buf[0] |= 0x80;

    return nlen + len;
}
