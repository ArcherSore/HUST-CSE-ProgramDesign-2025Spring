#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <functional>
#include <string>

namespace Common {
    // 堆排序模板函数
    // comp为自定义比较函数
    template<typename T>
    void heapify(std::vector<T*> &arr, std::function<bool(const T*, const T*)> comp);
    
    // 计算data的HASH值，返回HEX字符串
    std::string calculateHash(const std::vector<unsigned char> &data);

    // 加密函数
    unsigned char encrypt(unsigned char b);
    // 解密函数
    unsigned char decrypt(unsigned char b);
};

#include "common_impl.h"

#endif // COMMON_H