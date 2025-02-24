#include "decompressor.h"
#include "common.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// 定义命名空间 Trie，用于构建字典树（Trie）解码时使用
namespace Trie {
    // 字典树节点结构体，用于存储每个节点的信息
    struct Node {
        unsigned char value;  // 叶子节点存储的字节值
        bool isLeaf;          // 标记是否为叶子节点
        Node* left;           // 指向左子节点（代表编码中的 '0'）
        Node* right;          // 指向右子节点（代表编码中的 '1'）

        // 构造函数，初始化成员变量
        Node() : value(0), isLeaf(false), left(nullptr), right(nullptr) {}
    };

    // 函数: insert
    // 作用: 将指定的哈夫曼编码（字符串形式）和对应的字节值插入到字典树中
    //
    // 参数:
    //    root      - 字典树根节点
    //    code      - 哈夫曼编码字符串（由 '0' 和 '1'组成）
    //    byteValue - 编码对应的字节值
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

    // 函数: free
    // 作用: 递归释放整个字典树中分配的内存空间
    void free(Node* root) {
        if (!root) {
            return;
        }
        free(root->left);
        free(root->right);
        delete root;
    }
}

namespace TrieDecompressor {
    // 函数: decompressFile
    // 用途: 使用字典树方式（Trie）解压压缩文件，主要步骤：
    //       1. 记录解压开始时间
    //       2. 读取编码表（test/code.txt）并构建字典树
    //       3. 读取压缩文件内容
    //       4. 逐位遍历压缩数据，通过字典树进行解码
    //       5. 若设置了解密，则进行解密处理
    //       6. 校验收发人信息（与文件中存储信息比较）
    //       7. 将解压后的数据写入输出文件
    //       8. 显示解压过程的 HASH 值、数据大小及解压所耗时间
    //
    // 参数:
//    compressedFile - 压缩文件路径
//    senderInfo     - 发送者信息（用于校验）
//    receiverInfo   - 接收者信息（用于校验）
//    decrypt        - 是否需要解密
//    key            - 解密密钥
    void decompressFile(const std::string &compressedFile,
                        const std::string &senderInfo,
                        const std::string &receiverInfo,
                        bool decrypt,
                        const std::string &key) {
        // 1. 记录解压缩开始时间
        auto startTime = std::chrono::high_resolution_clock::now();

        // 2. 读取编码表，构建字典树用于解码
        std::string encodedPath = "test/code.txt";
        std::ifstream encodedTable(encodedPath);
        if (!encodedTable) {
            std::cerr << "Error opening encoded table file: " << encodedPath << std::endl;
            return;
        }
        std::string line;
        std::getline(encodedTable, line);
        // 第一行为原始文本字节长度
        int TextLength = std::stoi(line);
        
        // 创建字典树的根节点
        Trie::Node *root = new Trie::Node();
        // 逐行读取编码表并插入到字典树中
        while (std::getline(encodedTable, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string byteCodeStr; // 含 "0x" 前缀的字节码
            std::string lengthStr;   // 含 "0x" 前缀的编码长度
            ss >> byteCodeStr >> lengthStr;
            byteCodeStr = byteCodeStr.substr(2);
            lengthStr = lengthStr.substr(2);
            unsigned char byteCode = static_cast<unsigned char>(std::stoi(byteCodeStr, nullptr, 16));
            int length = std::stoi(lengthStr, nullptr, 16);
            // 读取编码字节，将其转换为二进制字符串形式
            std::string bitCodedStr;
            std::string bitStream;
            while (ss >> bitCodedStr) {
                bitCodedStr = bitCodedStr.substr(2);
                std::string byteStr = Common::hexToBinary(bitCodedStr);
                bitStream += byteStr;
            }
            // 截取前 length 位作为真正的哈夫曼编码
            std::string huffmanCode = bitStream.substr(0, length);
            Trie::insert(root, huffmanCode, byteCode);
        }
        encodedTable.close();

        // 3. 读取压缩文件数据
        std::ifstream compressedData(compressedFile, std::ios::binary);
        if (!compressedData) {
            std::cerr << "Error opening compressed file: " << compressedFile << std::endl;
            return;
        }
        std::vector<unsigned char> compressedContent;
        compressedContent.assign(std::istreambuf_iterator<char>(compressedData), std::istreambuf_iterator<char>());
        compressedData.close();

        // 4. 解码压缩数据：逐位判断，利用字典树确定对应的原始字节
        std::vector<unsigned char> decodedBytes;
        Trie::Node *current = root;
        for (unsigned char byte : compressedContent) {
            for (int pos = 7; pos >= 0 && decodedBytes.size() < TextLength; --pos) {
                int bit = (byte >> pos) & 1;
                if (bit == 0) {
                    current = current->left;
                } else {
                    current = current->right;
                }
                // 达到叶子节点则得到一个完整的字节
                if (current->isLeaf) {
                    decodedBytes.push_back(current->value);
                    current = root;
                }
            }
            if (decodedBytes.size() == TextLength) {
                break;
            }
        }

        // 5. 根据参数进行解密处理
        std::vector<unsigned char> processedBytes = decodedBytes;
        if (decrypt) {
            Common::decrypt(processedBytes, key);
        }

        // 6. 校验文件中存储的发送者和接收者信息，确保一致
        std::string fullDecoded(processedBytes.begin(), processedBytes.end());
        std::istringstream iss(fullDecoded);
        if (!senderInfo.empty()) {
            std::string sender;
            std::getline(iss, sender);
            if (sender != senderInfo) {
                std::cerr << "Sender info mismatch: " << senderInfo << std::endl;
                return;
            }
            std::cout << "Sender info: " << sender << std::endl;
        }
        if (!receiverInfo.empty()) {
            std::string receiver;
            std::getline(iss, receiver);
            if (receiver != receiverInfo) {
                std::cerr << "Receiver info mismatch: " << receiverInfo << std::endl;
                return;
            }
            std::cout << "Receiver info: " << receiver << std::endl;
        }

        // 7. 将解压后的数据写入输出文件，文件名格式为 "原文件名_j.txt"
        std::string outputFile = "test/" + Common::extractFileName(compressedFile) + "_j.txt";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFile << std::endl;
            return;
        }
        outFile.write(reinterpret_cast<const char *>(processedBytes.data()), processedBytes.size());
        outFile.close();

        // 释放字典树内存
        Trie::free(root);

        // 8. 显示解压后的数据 HASH、数据大小等信息
        std::string decompressedDataHash = Common::calculateHash(processedBytes);
        std::cout << "Decompressed data hash: 0x" << decompressedDataHash << std::endl;
        std::cout << "Decompressed data size: " << processedBytes.size() << std::endl;

        // 9. 记录结束时间，计算解压所用时间（毫秒）
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "01Trie decompression completed in " << duration.count() << "ms" << std::endl;

        // 10. 计算并显示压缩率：压缩文件大小与原文件大小的比例
        double compressionRatio = static_cast<double>(compressedContent.size()) / processedBytes.size();
        std::cout << "Compression ratio: " << compressionRatio << std::endl << std::endl;
    }
}

namespace HashDecompressor {
    // 函数: decompressFile
    // 用途: 使用哈希映射方式解压文件，其步骤类似于字典树解码，不过构建的是从编码串到字节值的哈希映射
    //
    // 参数:
//    compressedFile - 压缩文件路径
//    senderInfo     - 发送者信息（用于校验）
//    receiverInfo   - 接收者信息（用于校验）
//    decrypt        - 是否需要解密
//    key            - 解密密钥
    void decompressFile(const std::string &compressedFile,
                        const std::string &senderInfo,
                        const std::string &receiverInfo,
                        bool decrypt,
                        const std::string &key) {
        // 1. 记录解压开始时间
        auto startTime = std::chrono::high_resolution_clock::now();

        // 2. 读取编码表文件，构建哈希映射：键为哈夫曼编码字符串，值为对应的字节
        std::string encodedPath = "test/code.txt";
        std::ifstream encodedTable(encodedPath);
        if (!encodedTable) {
            std::cerr << "Error opening encoded table file: " << encodedPath << std::endl;
            return;
        }
        std::string line;
        std::getline(encodedTable, line);
        int TextLength = std::stoi(line);  // 获取原始文本字节长度
        
        std::unordered_map<std::string, unsigned char> codeMap;
        while (std::getline(encodedTable, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string byteCodeStr;
            std::string lengthStr;
            ss >> byteCodeStr >> lengthStr;
            byteCodeStr = byteCodeStr.substr(2);
            lengthStr = lengthStr.substr(2);
            unsigned char byteCode = static_cast<unsigned char>(std::stoi(byteCodeStr, nullptr, 16));
            int length = std::stoi(lengthStr, nullptr, 16);
            std::string bitCodedStr;
            std::string bitStream;
            while (ss >> bitCodedStr) {
                bitCodedStr = bitCodedStr.substr(2);
                std::string byteStr = Common::hexToBinary(bitCodedStr);
                bitStream += byteStr;
            }
            std::string huffmanCode = bitStream.substr(0, length);
            codeMap[huffmanCode] = byteCode;
        }
        encodedTable.close();

        // 3. 读取压缩文件内容
        std::ifstream compressedData(compressedFile, std::ios::binary);
        if (!compressedData) {
            std::cerr << "Error opening compressed file: " << compressedFile << std::endl;
            return;
        }
        std::vector<unsigned char> compressedContent;
        compressedContent.assign(std::istreambuf_iterator<char>(compressedData), std::istreambuf_iterator<char>());
        compressedData.close();

        // 4. 解码压缩数据：逐位构建编码串，匹配哈希映射得到对应字节
        std::vector<unsigned char> decodedBytes;
        std::string buffer;
        for (unsigned char byte : compressedContent) {
            for (int pos = 7; pos >= 0 && decodedBytes.size() < TextLength; --pos) {
                int bit = (byte >> pos) & 1;
                buffer += (bit == 0 ? '0' : '1');
                auto it = codeMap.find(buffer);
                if (it != codeMap.end()) {
                    decodedBytes.push_back(it->second);
                    buffer.clear();
                }
            }
            if (decodedBytes.size() == TextLength) {
                break;
            }
        }

        // 5. 根据需要进行解密处理
        std::vector<unsigned char> processedBytes = decodedBytes;
        if (decrypt) {
            Common::decrypt(processedBytes, key);
        }

        // 6. 校验发送者、接收者信息是否与输入一致
        std::string fullDecoded(processedBytes.begin(), processedBytes.end());
        std::istringstream iss(fullDecoded);
        if (!senderInfo.empty()) {
            std::string sender;
            std::getline(iss, sender);
            if (sender != senderInfo) {
                std::cerr << "Sender info mismatch: " << senderInfo << std::endl;
                return;
            }
            std::cout << "Sender info: " << sender << std::endl;
        }
        if (!receiverInfo.empty()) {
            std::string receiver;
            std::getline(iss, receiver);
            if (receiver != receiverInfo) {
                std::cerr << "Receiver info mismatch: " << receiverInfo << std::endl;
                return;
            }
            std::cout << "Receiver info: " << receiver << std::endl;
        }

        // 7. 将解码后的数据写入输出文件，文件名格式为 "原文件名_j.txt"
        std::string outputFile = "test/" + Common::extractFileName(compressedFile) + "_j.txt";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFile << std::endl;
            return;
        }
        outFile.write(reinterpret_cast<const char *>(processedBytes.data()), processedBytes.size());
        outFile.close();

        // 8. 显示解码后数据的 HASH 和大小
        std::string decompressedDataHash = Common::calculateHash(processedBytes);
        std::cout << "Decompressed data hash: 0x" << decompressedDataHash << std::endl;
        std::cout << "Decompressed data size: " << processedBytes.size() << std::endl;

        // 9. 记录结束时间，计算解压用时（毫秒）
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "Hash decompression completed in " << duration.count() << "ms" << std::endl;

        // 10. 计算并显示压缩比率
        double compressionRatio = static_cast<double>(compressedContent.size()) / processedBytes.size();
        std::cout << "Compression ratio: " << compressionRatio << std::endl << std::endl;
    }
}