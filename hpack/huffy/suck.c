#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct bitbuf {
    uint8_t octet, next, next_bits;
};

#ifndef LIKELY
# define LIKELY(x) __builtin_expect(x, 1)
#endif
#ifndef UNLIKELY
# define UNLIKELY(x) __builtin_expect(x, 0)
#endif

// discard_bits must be [1,8]
void bb_load_next(struct bitbuf *bb, uint8_t discard_bits, const uint8_t **buf, const uint8_t *end) {
    bb->octet <<= discard_bits;
    if (LIKELY(bb->next_bits <= discard_bits)) {
        // load another octet
        uint8_t loaded;
        if (UNLIKELY(*buf == end)) {
            // EOS bits
            loaded = 0xff;
        } else {
            loaded = **buf;
            ++*buf;
        }
        bb->octet |= bb->next >> (8 - discard_bits);
        bb->octet |= loaded >> (8 - discard_bits + bb->next_bits);
        bb->next = loaded << (discard_bits - bb->next_bits);
        bb->next_bits = 8 - (discard_bits - bb->next_bits);
    } else {
        // There are enough bits to shift into octet without loading an octet.
        bb->octet |= bb->next >> (8 - discard_bits);
        bb->next <<= discard_bits;
        bb->next_bits -= discard_bits;
    }
}

struct bb_reader {
    struct bitbuf bb;
    const uint8_t *pos, *end;
};

void bb_reader_init(struct bb_reader *bbr, const uint8_t *buf, size_t buflen) {
    memset(&bbr->bb, 0, sizeof(bbr->bb));
    bbr->bb.next_bits = 8; // push through the first non-input octet
    bbr->pos = buf;
    bbr->end = buf + buflen;

    // load both `octet` & `next` since we're on an octet boundary.
    // (one call would only load `next`)
    bb_load_next(&bbr->bb, 8, &bbr->pos, bbr->end);
    bb_load_next(&bbr->bb, 8, &bbr->pos, bbr->end);
}

__attribute__((const))
uint8_t bb_peek(const struct bb_reader *bbf) {
    return bbf->bb.octet;
}

uint8_t bb_next(struct bb_reader *bbr, uint8_t seek_bits) {
    bb_load_next(&bbr->bb, seek_bits, &bbr->pos, bbr->end);
    return bbr->bb.octet;
}

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
