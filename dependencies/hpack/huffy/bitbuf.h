#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct bitbuf {
    uint8_t octet, next, next_bits;
    // Only meaningful when pos == end in the bb_reader
    // Set to -1 as sentinel until then.
    // When zero, EOS has been reached, however it might otherwise be
    // >0 with all bits set to one when EOS is reached.
    int8_t eos_after_bits;
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

__attribute__((__const__))
bool bb_eos(const struct bb_reader *bbf);

#ifdef __cplusplus
} // extern "C"
#endif
