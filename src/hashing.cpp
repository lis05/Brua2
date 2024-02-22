#include "hashing.hpp"

namespace Hashing {
    //https://sair.synerise.com/efficient-integer-pairs-hashing/
    uint64_t SimpleHash(uint64_t x, uint64_t y) {
        if (x >= y) return y * y  + x;
        return x * x + x + y;
    }
    uint64_t splitmix64(uint64_t x) {
        x = x + 0x9E3779B97F4A7C15ul;
        x ^= x >> 30;
        x *= 0xBF58476D1CE4E5B9ul;
        x ^= x >> 27;
        x *= 0x94D049BB133111EBul;
        x ^= x >> 31;
        return x;
    }
    uint64_t Hash(uint64_t x, uint64_t y) {
        return splitmix64(SimpleHash(x, y));
    }
}