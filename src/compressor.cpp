#include "compressor.h"
#include "common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <functional>
#include <iomanip>
#include <algorithm>
#include <string>

// 使用匿名命名空间封装内部辅助函数，这样避免外部直接调用
namespace {
    // 函数: getHuffmanCode
    // 作用: 遍历哈夫曼树并生成各字节对应的哈夫曼编码
    //
    // 参数:
//    root      - 当前节点指针
//    code      - 当前路径编码（由 "0" 和 "1" 组成）
//    codeTable - 编码表数组，索引为字节值，值为该字节的编码字符串
    void getHuffmanCode(Compressor::Node *root, const std::string &code, std::vector<std::string> &codeTable) {
        if (!root) {
            return;
        }
        // 如果为叶子节点，则保存该字节的编码
        if (!root->left && !root->right) {
            codeTable[root->byteVal] = code;
            return;
        }
        // 递归遍历左子树（编码末尾加 "0"）
        getHuffmanCode(root->left, code + "0", codeTable);
        // 递归遍历右子树（编码末尾加 "1"）
        getHuffmanCode(root->right, code + "1", codeTable);
    }
    
    // 函数: computeWPL
    // 作用: 计算哈夫曼树的带权路径长度（WPL），即各叶子节点频率与其深度乘积的和
    //
    // 参数:
//    root  - 当前节点指针
//    depth - 当前节点的深度
//    wpl   - 累积的带权路径长度
    void computeWPL(Compressor::Node *root, int depth, int &wpl) {
        if (!root) {
            return;
        }
        // 如果为叶子节点，累加贡献值
        if (!root->left && !root->right) {
            wpl += root->freq * depth;
            return;
        }
        // 递归统计左右子树
        computeWPL(root->left, depth + 1, wpl);
        computeWPL(root->right, depth + 1, wpl);
    }
    
    // 函数: deleteTree
    // 作用: 递归释放整个哈夫曼树中分配的内存
    void deleteTree(Compressor::Node *root) {
        if (!root) {
            return;
        }
        deleteTree(root->left);
        deleteTree(root->right);
        delete root;
    }
}

namespace Compressor {
    // 辅助结构体: EncodedData
    // 用于存储每个字节的编码结果，包括原始字节、本编码长度及按8位分组的编码字节数组
    struct EncodedData {
        unsigned char byteCode;              // 对应字节
        int length;                          // 编码位数
        std::vector<unsigned char> bitCoded; // 存放编码得到的字节数组（每8位为1字节）
    };
    
    // 函数: compressFile
    // 用途: 对指定文件进行压缩，执行以下主要步骤：
    //       1. 读取原文件内容
    //       2. 插入发送者和接收者信息到文件内容中
    //       3. 若需要，对数据进行加密处理
    //       4. 统计各字节出现频率
    //       5. 构建哈夫曼树，并生成对应的编码表
    //       6. 计算原始数据的 HASH 值
    //       7. 输出编码表到 code.txt 文件
    //       8. 根据哈夫曼编码生成压缩数据（按位打包）
    //       9. 计算压缩数据的 HASH 值，并写入压缩文件
    //       10. 显示压缩数据的最后16个字节（调试信息）
    //       11. 释放哈夫曼树所占内存
    //
    // 参数:
//    inputFile    - 输入文件路径
//    senderInfo   - 发送者信息
//    receiverInfo - 接收者信息
//    encrypt      - 是否启用加密（默认为 false）
//    key          - 加密密钥（默认为空字符串）
    void compressFile(const std::string &inputFile,
                      const std::string &senderInfo,
                      const std::string &receiverInfo,
                      bool encrypt,
                      const std::string &key) {
        // 1. 读取文件内容到 vector 中
        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening input file: " << inputFile << std::endl;
            return;
        }
        std::vector<unsigned char> content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();
        
        // 2. 插入扩展信息：发送者信息和接收者信息，并以换行符分隔
        std::vector<unsigned char> processedContent;
        if (!senderInfo.empty()) {
            processedContent.insert(processedContent.end(), senderInfo.begin(), senderInfo.end());
            processedContent.push_back('\n');
        }
        if (!receiverInfo.empty()) {
            processedContent.insert(processedContent.end(), receiverInfo.begin(), receiverInfo.end());
            processedContent.push_back('\n');
        }
        // 插入原始文件内容
        processedContent.insert(processedContent.end(), content.begin(), content.end());

        // 3. 将插入扩展信息后的数据写回原文件（覆盖原数据）
        std::ofstream newinputFile(inputFile, std::ios::binary);
        if (!newinputFile) {
            std::cerr << "Error opening output file: " << inputFile << std::endl;
            return;
        }
        newinputFile.write(reinterpret_cast<const char*>(processedContent.data()), processedContent.size());
        newinputFile.close();

        // 4. 如果启用了加密，则对数据进行加密处理
        if (encrypt) {
            Common::encrypt(processedContent, key);
        }

        // 5. 统计各字节出现频率
        std::vector<int> freq(256, 0);
        for (unsigned char c : processedContent) {
            freq[c]++;
        }
        
        // 6. 构造出现的字节节点数组，用于构建哈夫曼树
        std::vector<Node *> nodes;
        for (int i = 0; i < 256; i++) {
            if (freq[i] == 0) {
                continue;
            }
            nodes.push_back(new Node(static_cast<unsigned char>(i), freq[i]));
        }

        // 7. 使用公共模块的堆排序对节点数组排序（主要根据频率，频率相同则根据字节大小）
        auto comp = [](const Node *a, const Node *b) -> bool {
            if (a->freq != b->freq) {
                return a->freq < b->freq;
            }
            return a->byteVal < b->byteVal;
        };
        Common::heapSort(nodes, comp);
        // 显示排序后的词频统计表（用于调试）
        std::cout << "*****Sorted Frequency List*****" << std::endl;
        std::cout << "Byte  Freq" << std::endl;
        for (auto n : nodes) {
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) 
                      << std::setfill('0') << static_cast<int>(n->byteVal);
            std::cout << '\t' << std::dec << n->freq << std::endl;
        }

        // 8. 使用小根堆构建哈夫曼树：不断合并节点，直至堆中只剩一个节点（即树根）
        Common::MinHeap<Node *, decltype(comp)> heap(comp);
        for (auto node : nodes) {
            heap.push(node);
        }
        while (heap.size() > 1) {
            Node *left = heap.top();
            heap.pop();
            Node *right = heap.top();
            heap.pop();
            Node *merged = new Node(left, right);
            heap.push(merged);
        }
        Node *huffmanTreeRoot = nodes.empty() ? nullptr : heap.top();

        // 9. 计算并显示哈夫曼树的总带权路径长度（WPL）
        int wpl = 0;
        computeWPL(huffmanTreeRoot, 0, wpl);
        std::cout << "********************************" << std::endl;
        std::cout << "Huffman Tree WPL: " << wpl << std::endl;

        // 10. 遍历哈夫曼树，生成每个字节的哈夫曼编码
        std::vector<std::string> huffmanCodes(256);
        getHuffmanCode(huffmanTreeRoot, "", huffmanCodes);

        // 11. 计算并显示原始数据（未压缩）的 HASH 值
        std::cout << "********************************" << std::endl;
        std::string OriginalDataHash = Common::calculateHash(content);
        std::cout << "Original Data Hash: 0x" << OriginalDataHash << std::endl;
        std::cout << "Original Data Size: " << processedContent.size() << " bytes" << std::endl;

        // 12. 构造编码表并输出到 code.txt 文件
        // 每行记录：字节码、编码长度以及编码字节（按8位分组）
        std::vector<EncodedData> EncodedTable(256);
        for (int i = 0; i < 256; i++) {
            if (freq[i] == 0) {
                continue;
            }
            EncodedTable[i].byteCode = static_cast<unsigned char>(i);
            EncodedTable[i].length = huffmanCodes[i].size();
            for (int j = 0; j < huffmanCodes[i].size(); j += 8) {
                std::string subStr = huffmanCodes[i].substr(j, 8);
                // 不足8位则补零
                while (subStr.size() < 8) {
                    subStr += "0";
                }
                unsigned char byte = 0;
                for (char bit : subStr) {
                    byte = byte << 1;
                    byte |= (bit == '1' ? 1 : 0);
                }
                EncodedTable[i].bitCoded.push_back(byte);
            }
        }
        std::string outputEncodingTable = "test/code.txt";
        std::ofstream tableFile(outputEncodingTable);
        if (!tableFile) {
            std::cerr << "Error opening output file: " << outputEncodingTable << std::endl;
            return;
        }
        // 第一行为原始文本字节长度
        tableFile << processedContent.size() << std::endl;
        for (auto &entry : EncodedTable) {
            if (entry.length == 0) {
                continue;
            }
            tableFile << "0x" << std::hex << std::uppercase << std::setw(2) 
                      << std::setfill('0') << static_cast<int>(entry.byteCode);
            tableFile << " 0x" << std::hex << std::uppercase << std::setw(2) 
                      << std::setfill('0') << entry.length;
            for (unsigned char byte : entry.bitCoded) {
                tableFile << " 0x" << std::hex << std::uppercase << std::setw(2) 
                          << std::setfill('0') << static_cast<int>(byte);
            }
            tableFile << std::dec << std::endl;
        }
        tableFile.close();

        // 13. 生成压缩数据：将每个字节的哈夫曼编码按位打包
        std::vector<unsigned char> compressedData;
        unsigned char byte = 0;
        int bitcount = 0;
        for (unsigned char c : processedContent) {
            const std::string &code = huffmanCodes[c];
            for (char bit : code) {
                byte = (byte << 1) | (bit == '1' ? 1 : 0);
                bitcount++;
                if (bitcount == 8) {
                    compressedData.push_back(byte);
                    byte = 0;
                    bitcount = 0;
                }
            }
        }
        // 补齐最后不足8位的数据（低位补0）
        if (bitcount > 0) {
            byte <<= (8 - bitcount);
            compressedData.push_back(byte);
        }
        
        // 14. 显示压缩数据的 HASH 值及文件大小（调试用）
        std::cout << "********************************" << std::endl;
        std::string CompressedDataHash = Common::calculateHash(compressedData);
        std::cout << "Compressed Data Hash: 0x" << CompressedDataHash << std::endl;
        std::cout << "Compressed Data Size: " << compressedData.size() << " bytes" << std::endl;

        // 15. 将压缩数据写入输出文件，文件名格式：原文件名.hfm
        std::string outputCompressedFile = "test/" + Common::extractFileName(inputFile) + ".hfm";
        std::ofstream outFile(outputCompressedFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputCompressedFile << std::endl;
            return;
        }
        outFile.write(reinterpret_cast<const char *>(compressedData.data()), compressedData.size());
        outFile.close();

        // 16. 显示压缩数据的最后 16 个字节（便于调试查看数据尾部）
        std::cout << "********************************" << std::endl;
        std::cout << "Last 16 Bytes of Compressed Data:" << std::endl;
        int startPos = std::max(0, static_cast<int>(compressedData.size()) - 16);
        for (int i = startPos; i < compressedData.size(); i++) {
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) 
                      << std::setfill('0') << static_cast<int>(compressedData[i]) << " ";
        }
        std::cout << std::dec << std::endl;
        std::cout << "********************************" << std::endl;

        // 17. 释放为构造哈夫曼树而申请的所有内存
        deleteTree(huffmanTreeRoot);
    }
}