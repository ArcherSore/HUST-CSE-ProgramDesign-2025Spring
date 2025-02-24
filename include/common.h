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
    // FNV-1a 64位哈希初始值与质数因子定义
    constexpr uint64_t FNV1A_64_INIT = 0xcbf29ce484222325ULL;
    constexpr uint64_t FNV1A_64_PRIME = 0x100000001b3ULL;

    // 模板函数: fnv1a_64
    // 用途: 计算数据的 FNV-1a 64 位哈希值
    // 参数:
    //    data - 可迭代数据（例如 vector 或字符串）
    // 返回:
    //    计算得到的 64 位哈希值
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
    // 模板函数: calculateHash
    // 用途: 计算数据的哈希值，并返回其16进制字符串格式表示
    // 参数:
    //    data - 任意支持迭代的数据容器
    // 返回:
    //    16进制字符串表示的哈希值
    template<typename T>
    inline std::string calculateHash(const T &data) {
        uint64_t hashValue = fnv1a_64(data);
        std::stringstream ss;
        ss << std::hex << hashValue;
        return ss.str();
    }

    // 堆排序: heapify 函数
    // 用途: 对存储指针的 vector 进行堆化处理，维护堆性质
    // 参数:
//    arr    - 待排序的指针数组
//    n      - 数组长度
//    i      - 当前堆化的起始索引
//    Compare- 比较函数，用于判断节点大小
    template<typename T, typename Compare>
    inline void heapify(std::vector<T *> &arr, int n, int i, Compare comp) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        // 检查左子节点
        if (left < n && comp(arr[largest], arr[left])) {
            largest = left;
        }
        // 检查右子节点
        if (right < n && comp(arr[largest], arr[right])) {
            largest = right;
        }
        // 若最大值改变，则交换并递归堆化
        if (largest != i) {
            std::swap(arr[i], arr[largest]);
            heapify(arr, n, largest, comp);
        }
    }

    // 堆排序: heapSort 函数
    // 用途: 对存储指针的 vector 进行堆排序
    // 参数:
//    arr    - 待排序的指针数组
//    Compare- 比较函数，指定排序规则
    template<typename T, typename Compare>
    inline void heapSort(std::vector<T *> &arr, Compare comp) {
        int n = arr.size();
        // 建堆：从最后一个非叶子节点开始堆化
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i, comp);
        }
        // 一个个取出堆顶元素，再调整堆结构
        for (int i = n - 1; i >= 0; i--) {
            std::swap(arr[0], arr[i]);
            heapify(arr, i, 0, comp);
        }
    }

    // 小根堆模板类，用于高效获取最小元素
    template<typename T, typename Compare = std::less<T>>
    class MinHeap {
    public:
        // 默认构造函数
        MinHeap() : comp(Compare()) {}

        // 带自定义比较函数的构造函数
        explicit MinHeap(Compare comparator) : comp(comparator) {}

        // 插入元素，并自下而上调整堆
        void push(const T& item) {
            data.push_back(item);
            siftUp(data.size() - 1);
        }

        // 弹出堆顶元素，并自上而下调整堆
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

        // 返回堆顶元素（引用）
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

        // 判断堆是否为空
        bool empty() const {
            return data.empty();
        }

        // 返回堆中元素的个数
        std::size_t size() const {
            return data.size();
        }

    private:
        std::vector<T> data;  // 存储堆数据
        Compare comp;         // 用于比较的函数对象

        // 自下而上调整：插入新元素后恢复堆结构
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

        // 自上而下调整：弹出堆顶元素后恢复堆结构
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

    // 声明加密处理函数
    void encrypt(std::vector<unsigned char> &data, const std::string &key);

    // 声明解密处理函数
    void decrypt(std::vector<unsigned char> &data, const std::string &key);

    // 获取文件名（不包含扩展名），例如 "test/example.txt" 返回 "example"
    std::string extractFileName(const std::string &filename);

    // 将十六进制字符串转换为二进制字符串（"A3" 转为 "10100011"）
    std::string hexToBinary(const std::string &hex);

    // 将单个字节转换为对应的8位二进制字符串
    std::string byteToBinary(unsigned char byte);
};

#endif // COMMON_H