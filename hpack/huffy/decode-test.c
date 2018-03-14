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
    char decoded[sizeof(input) * 8 / 5 + 1];

    ssize_t len = huffman_decode(input, sizeof(input), 8, decoded, sizeof(decoded) - 1);
    if (len >= 0)
        decoded[len] = '\0';

    if (strcmp(decoded, expect) != 0) {
        fprintf(stderr,
                "Decoded Huffman string to length %zd: %s, but expected length %zd: %s\n",
                len, decoded,
                sizeof(expect) - 1,
                expect);
        return 1;
    }

    return 0;
}
