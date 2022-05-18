#include <stdint.h>

// Future optimizations:
// - Remove `bits` from the structure and store it separately, keyed by input
//   character (saves 1 kiB; 256 bytes in `bits` and 768 bytes in padding).
// - Store static tables in native byte order (ie. swap for little-endian
//   machines) to avoid bswapping each multibyte symbol.
struct encv {
    uint_fast32_t value;
    uint_fast8_t bits;
};

extern const struct encv etable[256];
