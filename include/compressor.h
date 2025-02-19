#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <string>
#include <vector>

namespace Compressor {
    struct Node {
        unsigned char byteVal;
        int freq;
        Node *left, *right;

        Node(unsigned char b, int f) : byteVal(b), freq(f), left(nullptr), right(nullptr) {}
        // 节点合并
        Node(Node *l, Node *r) : freq(l->freq + r->freq), left(l), right(r) {
            byteVal = std::max(l->byteVal, r->byteVal);
        }
    };
    /*
        inputFile 待压缩文件名
        senderInfo 发送人信息
        receiverInfo 接收人信息
        encrypt 是否加密
    */
    void compressFile(const std::string &inputFile,
                      const std::string &senderInfo,
                      const std::string &receiverInfo,
                      bool encrypt);
}

#endif // COMPRESSOR_H