#pragma once

#include <stdint.h>

struct bitbuf {
    uint8_t octet, next, next_bits;
};

struct bb_reader {
    struct bitbuf bb;
    const uint8_t *pos, *end;
};

void bb_reader_init(struct bb_reader *bbr, const uint8_t *buf, size_t buflen);

__attribute__((__const__))
inline uint8_t bb_peek(const struct bb_reader *bbf) {
    return bbf->bb.octet;
}

uint8_t bb_next(struct bb_reader *bbr, uint8_t seek_bits);
