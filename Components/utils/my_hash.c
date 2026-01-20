#include "my_hash.h"

uint32_t my_hash(const char *str) {
    uint32_t hash = 5381;  // 初始值取质数，提升分布性
    char c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}