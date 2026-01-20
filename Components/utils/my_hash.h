#ifndef MY_HASH_H
#define MY_HASH_H

#include <stdint.h>

/**
 * @brief DJB2哈希算法实现
 * 
 * 适用于短字符串的高效哈希，具有较好分布性和低冲突率
 * 公式：hash = ((hash << 5) + hash) + c （等价于 hash * 33 + c）
 * 
 * @param str 输入字符串（以NULL结尾）
 * @return uint32_t 哈希值
 */
uint32_t my_hash(const char *str);

#endif /* MY_HASH_H */