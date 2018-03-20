#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "decode.h"
#include "encode.h"

#define INPUT_FILE "/dev/urandom"

static void dump(const uint8_t buf[], size_t len) {
    char fn[] = "testdataXXXXXX";
    int fd = mkstemp(fn);
    if (fd == -1) {
        fprintf(stderr, "Unable to dump test data\n");
        return;
    }

    ssize_t w = write(fd, buf, len);
    int err = close(fd);
    if (w != len || err != 0) {
        fprintf(stderr, "Failed to dump test data\n");
        return;
    }

    fprintf(stderr, "Test data written to %s\n", fn);
}

int main(void) {
    uint8_t data[1024];

    int fd = open(INPUT_FILE, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        perror("Failed to open " INPUT_FILE);
        return 0; // ... skipped
    }

    ssize_t sz = read(fd, data, sizeof(data));
    if (sz != sizeof(data)) {
        fprintf(stderr, "Insufficient random data to test\n");
        return 0; // ... skipped
    }

    close(fd);
    fd = -1;

    // Longest symbol is 30 bits.
    uint8_t encoded[sizeof(data) * 30 / 8 + 1];
    ssize_t encsz = huffman_encode(data, sizeof(data), encoded, sizeof(encoded));
    if (encsz < 0) {
        fprintf(stderr, "Failed to encode random bytes\n");
        return 1;
    }

    // cheat mode:
    // huffman_decode doesn't currently process the huffman bit or the
    // length prefix. Fortunately we can scan over them by skipping
    // all the high bits plus one octet without (which is the final
    // byte of the length prefix).

    size_t start = 0;
    while (encoded[start] & 0x80 && start < encsz)
        ++start;
    if (start == encsz) {
        fprintf(stderr, "Can't find end of length prefix?!\n");
        return 1;
    }

    ++start; // skip the last length-prefix octet.

    fprintf(stderr, "Skipping %zu length-prefix octets\n", start);

    uint8_t decoded[sizeof(data)];
    ssize_t decsz = huffman_decode(encoded + start, encsz - start, 8, decoded, sizeof(decoded));

    if (decsz != sizeof(data)) {
        fprintf(stderr, "Decode length mismatch; expected %zu but got %zd\n",
                sizeof(data), decsz);
        if (encsz > 0) {
            fprintf(stderr, "Dumping encoded buffer...\n");
            dump(encoded, encsz);
        }

        fprintf(stderr, "Dumping raw input...\n");
        dump(data, sizeof(data));
        return 1;
    }

    // This seems redundant but it's faster in the ideal case of the
    // test passing.
    if (memcmp(data, decoded, sizeof(data)) != 0) {
        // Find out which byte didn't enc/dec properly and dump
        // the test buffer for reproducibility.
        for (size_t i = 0; i < sizeof(data); ++i) {
            if (decoded[i] != data[i]) {
                fprintf(stderr, "Data mismatch at byte %zu\n", i + 1);

                dump(data, sizeof(data));

                return 1;
            }
        }
    }

    return 0;
}
