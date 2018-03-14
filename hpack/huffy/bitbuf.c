#include <string.h>

#include "bitbuf.h"

#ifndef LIKELY
# define LIKELY(x) __builtin_expect(x, 1)
#endif
#ifndef UNLIKELY
# define UNLIKELY(x) __builtin_expect(x, 0)
#endif

// discard_bits must be [1,8]
static void bb_load_next(struct bitbuf *bb, uint8_t discard_bits, const uint8_t **buf, const uint8_t *end) {
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

// ! This function is const !
uint8_t bb_peek(const struct bb_reader *bbf);

uint8_t bb_next(struct bb_reader *bbr, uint8_t seek_bits) {
    bb_load_next(&bbr->bb, seek_bits, &bbr->pos, bbr->end);
    return bbr->bb.octet;
}
