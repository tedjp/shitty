#include <stdio.h>
#include <string.h>

#include "decode.h"

// https://tools.ietf.org/html/rfc7541#appendix-C.6.1
static const uint8_t input[] = {
    0xd0, 0x7a, 0xbe, 0x94, 0x10, 0x54, 0xd4, 0x44,
    0xa8, 0x20, 0x05, 0x95, 0x04, 0x0b, 0x81, 0x66,
    0xe0, 0x82, 0xa6, 0x2d, 0x1b, 0xff };
static const char expect[] = "Mon, 21 Oct 2013 20:13:21 GMT";

int main(void) {
    uint8_t decoded[sizeof(input) * 8 / 5];

    ssize_t len = huffman_decode(input, sizeof(input), 8, decoded, sizeof(decoded) - 1);
    if (len < 0) {
        fprintf(stderr, "Failed to decode input\n");
        return 1;
    }

    if (memcmp(decoded, expect, len) != 0) {
        fprintf(stderr,
                "Decoded Huffman string to length %zd, but expected length %zd\n",
                len, sizeof(expect) - 1);
        fprintf(stderr, "byte  xpct  got\n");
        for (size_t i = 0; i < (len < sizeof(expect) - 1 ? len : sizeof(expect) - 1); ++i) {
            fprintf(stderr, "%4zu: 0x%02hhx 0x%02hhx\n", i + 1, expect[i], decoded[i]);
        }
        return 1;
    }

    return 0;
}
