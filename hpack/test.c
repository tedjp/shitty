#include <inttypes.h>
#include <stdio.h>

#include "number.h"

static int encode_tests(void) {
    uint8_t buf[123];

    // Examples from HPACK (RFC 7541)
    buf[0] = '\0';
    if (encode_number(10, 5, buf, sizeof(buf)) != 0) {
        fprintf(stderr, "failed to encode 10 into 5 bits");
        return 1;
    }

    if (buf[0] != 10) {
        fprintf(stderr, "failed to encode 10 into 5 bits as 10");
        return 1;
    }

    // Also test that top 3 bits are unchanged!


    if (encode_number(1337, 5, buf, sizeof(buf)) != 2) {
        fprintf(stderr, "Failed to encode 1337 into 2 extra bytes\n");
        return 1;
    }

    if (buf[0] != 31 || buf[1] != 154 || buf[2] != 10) {
        fprintf(stderr, "Failed to encode 1337 correctly\n");
        return 1;
    }

    // Check that short buffer is not overrun
    uint8_t shortbuf[1];
    if (encode_number(1337, 5, shortbuf, sizeof(shortbuf)) >= 0) {
        fprintf(stderr, "Erroneously able to encode 1337 with a 5-bit prefix\n");
        return 1;
    }

    if (encode_number(64, 5, shortbuf, sizeof(shortbuf)) >= 0) {
        fprintf(stderr, "Erroneously able to encode 64 with a 5-bit prefix\n");
        return 1;
    }

    // TODO: Other invalid inputs; prefix < 1 or > 8

    return 0;
}

static int decode_tests(void) {
    const struct {
        const uint8_t *bytes;
        size_t len;
        uint_fast8_t bits;
        uintmax_t expect;
    } inputs[] = {
        { (const uint8_t*)"\x0a", 1, 5, 10 },
        { (const uint8_t*)"\xea", 1, 5, 10 },
        { (const uint8_t*)"\x1f\x9a\x0a", 3, 5, 1337 },
    };

    int r = 0;

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        uintmax_t number = INTMAX_MAX;
        int err = decode_number(inputs[i].bytes, inputs[i].len, inputs[i].bits, &number);
        if (err) {
            fprintf(stderr, "Failed to decode input %zu\n", i + 1);
            r = err;
        }

        if (number != inputs[i].expect) {
            fprintf(stderr, "Wrong result decoding input %zu, expected %" PRIuMAX
                            ", got %" PRIuMAX "\n", i + 1, inputs[i].expect, number);
            r = -1;
        }
    }

    return r;
}

int main(void) {
    return encode_tests() == 0 && decode_tests() == 0 ? 0 : 1;
}
