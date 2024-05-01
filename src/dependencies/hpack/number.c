#include "error.h"
#include "number.h"

// Maximum number of octets to decode as a number, including the first
// which might be as short as 1 bit.
// 64-bit UINTMAX_MAX requires 10 octets:
// 9 full octets for 63 bits (63 ÷ 7 = 9) plus one bit = 10 octets.
#define MAX_OCTETS 10

__attribute__((const))
static uint8_t mask_from_bits(uint_fast8_t bits) {
    // Probably faster than a switch on 8 values
    // C++ could static_assert<> 1 <= bits <= 8?
    return 0xff >> (8 - bits);
}

// bits: how many of the least significant bits to use from the first octet.
// returns how many bytes were read (including the first)
// or negative error code
ptrdiff_t decode_number(const uint8_t* buf, size_t len, uint_fast8_t bits, uintmax_t *number) {
    if (len == 0 || bits > 8)
        return -PROTOCOL_ERROR;

    if (number == NULL || !buf)
        return -INTERNAL_ERROR;

    uint_fast8_t mask = mask_from_bits(bits);
    uint_fast8_t field = buf[0] & mask;

    *number = field;
    if (field != mask)
        return 1; // just used first octet

    uintmax_t mult = 1;

    for (size_t i = 1; i < len && i <= MAX_OCTETS; ++i) {
        field = buf[i] & 0x7f;
        *number += field * mult;

        if (!(buf[i] & 0x80)) {
            // Last field (most significant byte)
            return i + 1; // end of value
        }

        if (mult << 7 < mult) {
            // overflow
            return -INTERNAL_ERROR;
        }

        mult = mult << 7;
    }

    return -PROTOCOL_ERROR; // too big
}

// Return number of bytes used for encoding (including the bits in
// the prefix/first byte). Returns 1 for all values that fit in the prefix.
// buflen *includes* the length of the first byte.
ptrdiff_t encode_number(uintmax_t number, uint_fast8_t prefix_bits, uint8_t *buf, size_t buflen) {
    if (!buf || buflen == 0 || prefix_bits > 8 || prefix_bits < 1)
        return -1; // maybe -INTERNAL_ERROR

    uint8_t mask = mask_from_bits(prefix_bits);

    if (number < mask) {
        // Mask off any existing bits that might have been non-zero.
        buf[0] = (buf[0] & ~mask) | number;
        return 1;
    }

    buf[0] |= mask;

    number -= mask;

    ptrdiff_t octet = 0;
    while (number >= 0x80) {
        ++octet;

        if (octet >= buflen)
            return -1; // maybe -INTERNAL_ERROR or the calculated required buffer length?

        buf[octet] = 0x80 | number % 0x80;
        number /= 0x80;
    }

    ++octet;

    if (octet >= buflen)
        return -1; // maybe -INTERNAL_ERROR or the calculated required buffer length?

    buf[octet] = number;

    return octet + 1; // include the first, partial octet
}
