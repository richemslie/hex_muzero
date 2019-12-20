#include "basestate.h"

using namespace MuZero::SM;

/* The MIT License

fasthash.h and fasthash.c are the source code of FastHash.
Code here is derived from the above - copy Richard Emslie 2015.

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

// Compression function for Merkle-Damgard construction.
// This function is generated using the framework provided.
#define mix(h)                        \
    ({                                \
        (h) ^= (h) >> 23;             \
        (h) *= 0x2127599bf4325c37ULL; \
        (h) ^= (h) >> 47;             \
    })

std::size_t BaseState::Hasher::operator()(const BaseState* key) const {
    const uint64_t seed = 42 * 42 * 42 * 42;
    const uint64_t m = 0x880355f21e6d1965ULL;
    const uint64_t* pos = (const uint64_t*) key->data;
    const uint64_t* end = pos + (key->byte_count / 8);
    const unsigned char* pos2;
    uint64_t h = seed ^ (key->byte_count * m);
    uint64_t v;

    while (pos != end) {
        v = *pos++;
        h ^= mix(v);
        h *= m;
    }

    pos2 = (const unsigned char*) pos;
    v = 0;

    switch (key->byte_count & 7) {
    case 7:
        v ^= (uint64_t) pos2[6] << 48;
    case 6:
        v ^= (uint64_t) pos2[5] << 40;
    case 5:
        v ^= (uint64_t) pos2[4] << 32;
    case 4:
        v ^= (uint64_t) pos2[3] << 24;
    case 3:
        v ^= (uint64_t) pos2[2] << 16;
    case 2:
        v ^= (uint64_t) pos2[1] << 8;
    case 1:
        v ^= (uint64_t) pos2[0];
        h ^= mix(v);
        h *= m;
    }

    return mix(h);
}

std::size_t BaseState::HasherMasked::operator()(const BaseState* key) const {
    const int rem = key->byte_count % 8;

    // magics:
    const uint64_t seed = 42 * 42 * 42 * 42;
    const uint64_t m = 0x880355f21e6d1965ULL;
    uint64_t h = seed ^ (key->byte_count * m);

    // ptr to data/mask
    const uint64_t* pos = (const uint64_t*) key->data;
    const uint64_t* mask_pos = (const uint64_t*) this->mask_buf;

    // only need one end
    const uint64_t* end = pos + (key->byte_count / 8);

    uint64_t v = 0;

    // do 8 byte chunks
    while (pos != end) {
        uint64_t v = *pos++;

        // perform mask here
        v &= *mask_pos++;

        h ^= mix(v);
        h *= m;
    }

    v = 0;
    const uint8_t* byte_pos = (const uint8_t*) pos;
    const uint8_t* byte_mask_pos = (const uint8_t*) mask_pos;

    switch (rem) {
    case 7:
        v ^= (uint64_t) byte_pos[6] << 48;
        v &= (uint64_t) byte_mask_pos[6] << 48;
    case 6:
        v ^= (uint64_t) byte_pos[5] << 40;
        v &= (uint64_t) byte_mask_pos[5] << 40;
    case 5:
        v ^= (uint64_t) byte_pos[4] << 32;
        v &= (uint64_t) byte_mask_pos[4] << 32;
    case 4:
        v ^= (uint64_t) byte_pos[3] << 24;
        v &= (uint64_t) byte_mask_pos[3] << 24;
    case 3:
        v ^= (uint64_t) byte_pos[2] << 16;
        v &= (uint64_t) byte_mask_pos[2] << 16;
    case 2:
        v ^= (uint64_t) byte_pos[1] << 8;
        v &= (uint64_t) byte_mask_pos[1] << 8;
    case 1:
        v ^= (uint64_t) byte_pos[0];
        v &= (uint64_t) byte_mask_pos[0];
        h ^= mix(v);
        h *= m;
    }

    return mix(h);
}
