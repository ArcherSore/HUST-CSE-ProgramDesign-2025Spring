#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <vector>
#include <functional>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstdint>
#include <stdexcept>

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
    // 计算data的HASH值，返回16进制字符串
    template<typename T>
    inline std::string calculateHash(const T &data) {
        uint64_t hashValue = fnv1a_64(data);
        std::stringstream ss;
        ss << std::hex << hashValue;
        return ss.str();
    }

    // 堆排序模板函数实现
    template<typename T, typename Compare>
    inline void heapify(std::vector<T *> &arr, int n, int i, Compare comp) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        // 如果左孩子存在且满足比较条件，则更新largest
        if (left < n && comp(arr[largest], arr[left])) {
            largest = left;
        }
        // 如果右孩子存在且满足比较条件，则更新largest
        if (right < n && comp(arr[largest], arr[right])) {
            largest = right;
        }
        // 如果最大值不在当前节点，交换后继续递归
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
        // 依次将堆顶元素交换到队尾，并不断调整堆
        for (int i = n - 1; i >= 0; i--) {
            std::swap(arr[0], arr[i]);
            heapify(arr, i, 0, comp);
        }
    }

    // 小根堆模板类
    template<typename T, typename Compare = std::less<T>>
    class MinHeap {
    public:
        MinHeap() : comp(Compare()) {}

        explicit MinHeap(Compare comparator) : comp(comparator) {}

        void push(const T& item) {
            data.push_back(item);
            siftUp(data.size() - 1);
        }

        void pop() {
            if (empty()) {
                throw std::runtime_error("Heap is empty");
            }
            data[0] = data.back();
            data.pop_back();
            if (!empty()) {
                siftDown(0);
            }
        }

        T& top() {
            if (empty()) {
                throw std::runtime_error("Heap is empty");
            }
            return data[0];
        }

        const T& top() const {
            if (empty()) {
                throw std::runtime_error("Heap is empty");
            }
            return data[0];
        }

        bool empty() const {
            return data.empty();
        }

        std::size_t size() const {
            return data.size();
        }

    private:
        std::vector<T> data;
        Compare comp;

        // 自下而上调整
        void siftUp(std::size_t index) {
            while (index > 0) {
                std::size_t parent = (index - 1) / 2;
                if (comp(data[index], data[parent])) {
                    std::swap(data[index], data[parent]);
                    index = parent;
                } else {
                    break;
                }
            }
        }

        // 自上而下调整
        void siftDown(std::size_t index) {
            std::size_t n = data.size();
            while (true) {
                std::size_t smallest = index;
                std::size_t left = 2 * index + 1;
                std::size_t right = 2 * index + 2;
                if (left < n && comp(data[left], data[smallest])) {
                    smallest = left;
                }
                if (right < n && comp(data[right], data[smallest])) {
                    smallest = right;
                }
                if (smallest != index) {
                    std::swap(data[index], data[smallest]);
                    index = smallest;
                } else {
                    break;
                }
            }
        }
    };

    // 加密处理
    void encrypt(std::vector<unsigned char> &data, const std::string &key);

    // 解密处理
    void decrypt(std::vector<unsigned char> &data, const std::string &key);

    // 获取文件名（不包含扩展名）
    std::string extractFileName(const std::string &filename);

    // 将16进制字符串转换为二进制字符串
    std::string hexToBinary(const std::string &hex);

    // 将字节转换为8位二进制字符串
    std::string byteToBinary(unsigned char byte);
};

#endif // COMMON_H