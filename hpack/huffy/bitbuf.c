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
            if (bb->eos_after_bits == -1) {
                // first eos
                bb->eos_after_bits = 8 + bb->next_bits;
            }
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

    if (bb->eos_after_bits != -1) {
        if (discard_bits > bb->eos_after_bits)
            bb->eos_after_bits = 0;
        else
            bb->eos_after_bits -= discard_bits;
    }
}

// This function is __attribute__((const))
bool bb_eos(const struct bb_reader *bbr) {
    // EOS if either eos_after_bits == 0
    // or all bits are one.
    // Potentially have to check both the current octet and the next bits,
    // depending on how many bits there are to EOS.
    // In general, it's likely that only when eos_after_bits < 8
    // will it actually be EOS. Earlier bits likey to have zeroes.
    // Alternatively could check that pos != end
    if (LIKELY(bbr->bb.eos_after_bits == -1))
        return false;

    if (bbr->bb.eos_after_bits == 0)
        return true;

    // Otherwise check whether all bits are one

    if (bbr->bb.eos_after_bits > 8) {
        // Check next bits
        uint8_t mask = 0xff << (8 - bbr->bb.eos_after_bits % 8);
        if ((bbr->bb.next & mask) != mask)
            return false; // found a zero bit
    }

    uint8_t mask = 0xff << (8 - bbr->bb.eos_after_bits);
    return (bbr->bb.octet & mask) == mask;
}

void bb_reader_init(struct bb_reader *bbr, const uint8_t *buf, size_t buflen) {
    memset(&bbr->bb, 0, sizeof(bbr->bb));
    bbr->bb.eos_after_bits = -1;
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
