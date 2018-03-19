#include <stdbool.h>
#include <stdio.h>

#include "encode.h"

int main(void) {
    const uint8_t expect[] = {
        0x8c, 0xf1, 0xe3, 0xc2, 0xe5, 0xf2, 0x3a, 0x6b, 0xa0, 0xab, 0x90, 0xf4, 0xff
    };

    const uint8_t input[] = "www.example.com";
    uint8_t buf[123];

    ssize_t len = huffman_encode(input, sizeof(input) - 1, buf, sizeof(buf));

    if (len < 0) {
        fprintf(stderr, "huffman_encode failed\n");
        return 1;
    }

    bool ok = true;

    if (len != sizeof(expect)) {
        fprintf(stderr, "Got length %zu, expected %zu\n", len, sizeof(expect));
        ok = false;
        // continue to print output
    }

    // Bytewise comparison for better diagnostic output
    for (ssize_t i = 0; i < len; ++i) {
        if (expect[i] != buf[i]) {
            fprintf(stderr, "Encoded string differs at byte %zd: got 0x%02x but expected 0x%02x\n",
                    i + 1, buf[i], expect[i]);
            ok = false;
        }
    }

    return ok ? 0 : 1;
}
