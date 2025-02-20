#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include <vector>
#include <functional>
#include <algorithm>
#include <sstream>
#include <string>

namespace {
    constexpr uint64_t FNV1A_64_INIT = 0xcbf29ce484222325ULL;
    constexpr uint64_t FNV1A_64_PRIME = 0x100000001b3ULL;

    template<typename T>
    inline uint64_t fnv1a_64(const T &data) {
        uint64_t hash = FNV1A_64_INIT;
        for (unsigned char byte : data) {
            hash ^= byte;
            hash *= FNV1A_64_PRIME;
        }
        return hash;
    }
}

namespace Common {
    template<typename T>
    inline std::string calculateHash(const T &data) {
        uint64_t hashValue = fnv1a_64(data);
        std::stringstream ss;
        // 将HASH值转换为16进制字符串
        ss << std::hex << hashValue;
        return ss.str();
    }

    template<typename T, typename Compare>
    inline void heapify(std::vector<T *> &arr, int n, int i, Compare comp) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        // 如果左孩子存在且满足比较条件，更新largest
        if (left < n && comp(arr[left], arr[largest])) {
            largest = left;
        }
        // 如果右孩子存在且满足比较条件，更新largest
        if (right < n && comp(arr[right], arr[largest])) {
            largest = right;
        }
        // 如果最大值不在当前节点，交换
        if (largest != i) {
            std::swap(arr[i], arr[largest]);
            heapify(arr, n, largest, comp);
        }
    }

    template<typename T, typename Compare>
    inline void heapSort(std::vector<T *> &arr, Compare comp) {
        int n = arr.size();
        // 构建堆
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i, comp);
        }
        for (int i = n - 1; i >= 0; i--) {
            std::swap(arr[0], arr[i]);
            heapify(arr, i, 0, comp);
        }
    }
};

#endif // COMMON_IMPL_H