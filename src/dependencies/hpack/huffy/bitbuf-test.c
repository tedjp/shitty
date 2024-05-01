#include <stdbool.h>
#include <stdio.h>

#include "bitbuf.h"

// https://tools.ietf.org/html/rfc7541#appendix-C.4.1
static const uint8_t input[] = { 0xf1, 0xe3, 0xc2, 0xe5,
                                 0xf2, 0x3a, 0x6b, 0xa0,
                                 0xab, 0x90, 0xf4, 0xff };

static bool symbol_matches(uint8_t octet, uint8_t symbol, uint_fast8_t symbol_bits) {
    return octet >> (8 - symbol_bits) == symbol;
}

int main(void) {
    struct bb_reader reader;
    bb_reader_init(&reader, input, sizeof(input));

    const struct {
        uint8_t symbol, symbol_bits;
    } expect[] = {
        { 0x78, 7 }, // w
        { 0x78, 7 }, // w
        { 0x78, 7 }, // w
        { 0x17, 6 }, // .
        { 0x05, 5 }, // e
        { 0x79, 7 }, // x
        { 0x03, 5 }, // a
        { 0x29, 6 }, // m
        { 0x2b, 6 }, // p
        { 0x28, 6 }, // l
        { 0x05, 5 }, // e
        { 0x17, 6 }, // .
        { 0x04, 5 }, // c
        { 0x07, 5 }, // o
        { 0x29, 6 }, // m
    };

    bool ok = true;

    for (size_t i = 0; i < sizeof(expect) / sizeof(expect[0]); ++i) {
        uint8_t sym = bb_peek(&reader);
        if (!symbol_matches(sym, expect[i].symbol, expect[i].symbol_bits)) {
            fprintf(stderr, "Symbol %zu mismatch; got 0x%02hhx expected 0x%02hhx\n",
                    i + 1, sym, expect[i].symbol);
            ok = false;
        }
        bb_next(&reader, expect[i].symbol_bits);
    }

    // All remaining reads should return ffs (EOS)
    if (!symbol_matches(bb_peek(&reader), 0xff, 8)) {
        fprintf(stderr, "Reads at end did not return ffs\n");
        ok = false;
    }

    return ok ? 0 : 1;
}
