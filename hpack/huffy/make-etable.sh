#!/bin/sh
set -e

# Input: Static huffman table from RFC 7541 without empty lines
# or page boundaries.

sed -e 's,.\{48\} \+\([0-9a-f]\+\)  \[ \?\([0-9]\+\)\],\1\, \2,' < huff.txt |
awk -F' ' "
BEGIN { print \"static const struct encv etable[256] = {\" }
NR <= 256 { printf \"    { 0x%-9s %2s },\\n\", \$1, \$2 }
END { print \"};\" }"
