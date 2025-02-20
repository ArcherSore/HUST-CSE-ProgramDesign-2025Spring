#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <functional>
#include <string>

namespace Common {
    // 堆排序模板函数
    // comp为自定义比较函数
    template<typename T>
    inline void heapify(std::vector<T*> &arr, std::function<bool(const T*, const T*)> comp);
    
    // 计算data的HASH值，返回HEX字符串
    template<typename T>
    inline std::string calculateHash(const T &data);


    // 加密函数
    unsigned char encrypt(unsigned char b);
    // 解密函数
    unsigned char decrypt(unsigned char b);

    // 删除扩展名
    std::string extractFileName(const std::string &filename);

    // 将16进制字符串转换为二进制字符串
    std::string hexToBinary(const std::string &hex);

    // 将字节转换为8位二进制字符串
    std::string byteToBinary(unsigned char byte);
};

#include "common_impl.h"

#endif // COMMON_H