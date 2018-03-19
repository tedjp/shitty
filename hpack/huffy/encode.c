#include <limits.h>
#include <string.h>

#include "encode.h"
#include "../number.h"


// Future optimizations:
// - Remove `bits` from the structure and store it separately, keyed by input
//   character (saves 1 kiB; 256 bytes in `bits` and  768 bytes in padding).
// - Store static tables in native byte order (ie. swap for little-endian
//   machines) to avoid bswapping each multibyte symbol.
struct encv {
    uint_fast32_t value;
    uint_fast8_t bits;
};

#include "encode-table.c"

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

#define MAX_HUFF_STR 8192

ssize_t huffman_encode(const uint8_t *input, size_t input_len, uint8_t *buf, size_t buflen) {
    // Alternatively this could encode into a user-supplied buffer or just
    // allocate exactly the length of the non-coded input (since it will be
    // discarded if the Huffman coding is longer anyway).
    //
    // TODO: Alternative-alternative: Encode the string starting its output
    // at buf[1] in the hope that the encoded length will be <127 and
    // doesn't have to be copied. If it ends up having length >= 127,
    // memmove() the string after figuring out how long the length prefix is
    // (then set the length prefix).
    // FIXME: This is kind of a bad limitation.
    uint8_t huffstr[MAX_HUFF_STR];
    ssize_t len = encode(input, input_len, huffstr, sizeof(huffstr));

    if (len < 0)
        return len;

    buf[0] = 0x80; // set Huffman bit

    size_t stlen = (size_t) len;

    ssize_t nlen = encode_number(stlen, 7, buf, buflen);
    if (nlen < 0)
        return nlen;

    // Advance past the length prefix (+1 to advance past buf[0] which is not
    // included in encode_number's return value.
    buf += nlen;
    buflen -= nlen;

    if (__builtin_expect(stlen > buflen, 0)) {
        // No longer room for encoded string
        return -1;
    }

    memcpy(buf, huffstr, stlen);

    // Check for integer overflow before adding both lengths
    if (__builtin_expect(SSIZE_MAX - len < nlen, 0)) {
        return -1;
    }

    return nlen + len;
}
