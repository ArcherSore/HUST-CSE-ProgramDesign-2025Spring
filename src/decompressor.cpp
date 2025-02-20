#include "decompressor.h"
#include "common.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace Trie {
    struct Node {
        unsigned char value;
        bool isLeaf;
        Node* left;
        Node* right;

        Node() : value(0), isLeaf(false), left(nullptr), right(nullptr) {}
    };

    void insert(Node* root, const std::string &code, unsigned char byteValue) {
        Node* current = root;
        for (char c : code) {
            if (c == '0') {
                if (!current->left) {
                    current->left = new Node();
                }
                current = current->left;
            } else {
                if (!current->right) {
                    current->right = new Node();
                }
                current = current->right;
            }
        }
        current->isLeaf = true;
        current->value = byteValue;
    }

    void free(Node* root) {
        if (!root) {
            return;
        }
        free(root->left);
        free(root->right);
        delete root;
    }
}

namespace Decompressor {
    void decompressFile(const std::string &compressedFile, bool decrypt) {
        // 1. 记录解压缩开始时间
        auto startTime = std::chrono::high_resolution_clock::now();

        // 2. 读取编码表文件，并构建字典树
        std::ifstream encodedTable("build/output/code.txt");
        if (!encodedTable) {
            std::cerr << "Error opening encoded table file: /build/output/code.txt" << std::endl;
            return;
        }
        std::string line;
        std::getline(encodedTable, line);
        // 获取文本长度
        int TextLength = std::stoi(line);
        
        Trie::Node *root = new Trie::Node();
        while (std::getline(encodedTable, line)) {
            // 跳过最后的空行
            if (line.empty()) {
                continue;
            }

            std::stringstream ss(line);
            std::string byteCodeStr; // 字节码
            std::string lengthStr; // 编码长度
            ss >> byteCodeStr >> lengthStr;
            // 去除 0x 前缀，转换成十进制数
            // 获取字节码和编码长度
            byteCodeStr = byteCodeStr.substr(2);
            lengthStr = lengthStr.substr(2);
            unsigned char byteCode = static_cast<unsigned char>(std::stoi(byteCodeStr, nullptr, 16));
            int length = std::stoi(lengthStr, nullptr, 16);
            // 获取编码字节
            std::string bitCodedStr; // 位编码字节(字符串形式)
            std::string bitStream; // 编码表的十六进制位串
            while (ss >> bitCodedStr) {
                bitCodedStr = bitCodedStr.substr(2);
                std::string byteStr = Common::hexToBinary(bitCodedStr);
                bitStream += byteStr;
            }
            std::string huffmanCode = bitStream.substr(0, length);
            // 插入字典树
            Trie::insert(root, huffmanCode, byteCode);
        }
        encodedTable.close();

        // 3. 读取压缩文件
        std::ifstream compressedData(compressedFile, std::ios::binary);
        if (!compressedData) {
            std::cerr << "Error opening compressed file: " << compressedFile << std::endl;
            Trie::free(root);
            return;
        }
        std::vector<unsigned char> compressedContent; // 读入的压缩内容
        compressedContent.assign(std::istreambuf_iterator<char>(compressedData), std::istreambuf_iterator<char>());
        compressedData.close();

        // 4. 解码压缩数据
        std::vector<unsigned char> decodedBytes; // 解码后的字节数组
        Trie::Node *current = root;
        for (unsigned char byte : compressedContent) {
            // 逐位解码
            for (int pos = 7; pos >= 0 && decodedBytes.size() < TextLength; --pos) {
                int bit = (byte >> pos) & 1;
                if (bit == 0) {
                    current = current->left;
                } else {
                    current = current->right;
                }
                // 如果到达叶子节点，则解码出一个字符
                if (current->isLeaf) {
                    decodedBytes.push_back(current->value);
                    current = root;
                }
            }
            if (decodedBytes.size() == TextLength) {
                break;
            }
        }

        // 5. 解密处理
        if (decrypt) {
            for (unsigned char &ch : decodedBytes) {
                ch = Common::decrypt(ch);
            }
        }

        // 6. 输出解压缩后的文本
        std::string outputFile = "build/output/" + Common::extractFileName(compressedFile) + "_j.txt";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFile << std::endl;
            Trie::free(root);
            return;
        }
        outFile.write(reinterpret_cast<const char *>(decodedBytes.data()), decodedBytes.size());
        outFile.close();
        // 释放字典树内存
        Trie::free(root);

        // 7. 显示解压缩文本hash值
        std::string decompressedDataHash = Common::calculateHash(decodedBytes);
        std::cout << "Decompressed data hash: 0x" << decompressedDataHash << std::endl;
        std::cout << "Decompressed data size: " << decodedBytes.size() << std::endl;

        // 8. 记录解压缩结束时间
        auto endTime = std::chrono::high_resolution_clock::now();
        // 转换成ms
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Decompression completed in " << duration.count() << "ms" << std::endl;
    }
}