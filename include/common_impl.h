#ifndef COMMON_IMPL_H
#define COMMON_IMPL_H

#include <vector>
#include <functional>
#include <algorithm>

namespace Common {
    template<typename T, typename Compare>
    void heapify(std::vector<T *> &arr, int n, int i, Compare comp) {
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
    void heapSort(std::vector<T *> &arr, Compare comp) {
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