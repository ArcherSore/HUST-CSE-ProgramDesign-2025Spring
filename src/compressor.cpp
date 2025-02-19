#include "compressor.h"
#include "common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <functional>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <string>

namespace {
    // 生成哈夫曼编码表
    void getHuffmanCode(Compressor::Node *root, const std::string &code, std::vector<std::string> &codeTable) {
        if (!root) {
            return;
        }
        if (!root->left && !root->right) {
            codeTable[root->byteVal] = code;
            return;
        }
        getHuffmanCode(root->left, code + "0", codeTable);
        getHuffmanCode(root->right, code + "1", codeTable);
    }
    // 计算哈夫曼树的WPL
    void computeWPL(Compressor::Node *root, int depth, int &wpl) {
        if (!root) {
            return;
        }
        if (!root->left && !root->right) {
            wpl += root->freq * depth;
            return;
        }
        computeWPL(root->left, depth + 1, wpl);
        computeWPL(root->right,depth + 1, wpl);
    }
    // 释放哈夫曼树结点
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
    struct EncodedData {
        unsigned char byteCode;// 字节码
        int length; // 编码长度
        std::vector<unsigned char> bitCoded; // 位编码字节
    };
    void compressFile(const std::string &inputFile,
                      const std::string &senderInfo,
                      const std::string &receiverInfo,
                      bool encrypt) {
        // 1. 读取文本文件的内容
        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening input file: " << inputFile << std::endl;
            return;
        }
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string content = buffer.str();
        inFile.close();
        
        // 2. 插入扩展信息：发送人和接收人信息
        if (!senderInfo.empty()) {
            content = senderInfo + "\n" + content;
        }
        if (!receiverInfo.empty()) {
            content = content + "\n" + receiverInfo;
        }

        // 3. 加密处理
        std::string processedContent = content;
        if (encrypt) {
            for (char &ch : processedContent) {
                ch = static_cast<char>(Common::encrypt(static_cast<unsigned char>(ch)));
            }
        }

        // 4. 统计每个字节出现的频率
        std::vector<int> freq(256, 0);
        for (unsigned char c : processedContent) {
            freq[c]++;
        }
        
        // 5. 构造结点数组
        std::vector<Node *> nodes;
        for (int i = 0; i < 256; i++) {
            if (freq[i] == 0) {
                continue;
            }
            nodes.push_back(new Node(static_cast<unsigned char>(i), freq[i]));
        }

        // 6. 使用公共模块的堆排序堆结点数组排序
        auto comp = [](const Node *a, const Node *b) -> bool {
            if (a->freq != b->freq) {
                return a->freq > b->freq;
            }
            return a->byteVal > b->byteVal;
        };
        Common::heapSort(nodes, comp);
        // 显示排序后的词频统计表
        std::cout << "*****Sorted Frequency List*****" << std::endl;
        for (auto n : nodes) {
            /*
                hex->整数的输出设置为十六进制
                dec->整数的输出设置为十进制
                setw->设置输出字段宽度
                setfill->指定在宽度不足时左侧用字符'0'填充
            */
            std::cout << "Byte: 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(n->byteVal);
            std::cout << " Frequency: " << std::dec << n->freq << std::endl;
        }

        // 7. 构建哈夫曼树
        while (nodes.size() > 1) {
            Node *left = nodes[0];
            Node *right = nodes[1];
            Node *merged = new Node(left, right);

            nodes.erase(nodes.begin());
            nodes.erase(nodes.begin());
            nodes.push_back(merged);
            Common::heapSort(nodes, comp);
        }
        Node *huffmanTreeRoot = nodes.empty() ? nullptr : nodes[0];

        // 8. 计算WPL
        int wpl = 0;
        computeWPL(huffmanTreeRoot, 0, wpl);
        std::cout << "Huffman Tree WPL: " << wpl << std::endl;

        // 9. 遍历哈夫曼树得到每个字节对应的编码
        std::vector<std::string> huffmanCodes(256);
        getHuffmanCode(huffmanTreeRoot, "", huffmanCodes);
        // 显示生成的哈夫曼编码表
        // std::cout << "*****Huffman Encoding Table*****" << std::endl;
        // for (int i = 0; i < 256; i++) {
        //     if (freq[i] == 0) {
        //         continue;
        //     }
        //     std::cout << "Byte: 0x" << std::hex << std::uppercase std::setw(2) << std::setfill('0') << i;
        //     std::cout << " Code: " << huffmanCodes[i] << std::endl;
        // }

        // 10. 获取编码表并输出
        std::vector<EncodedData> EncodedTable(256);
        for (int i = 0; i < 256; i++) {
            if (freq[i] == 0) {
                continue;
            }
            // 获取字节码
            EncodedTable[i].byteCode = static_cast<unsigned char>(i);
            // 获取编码长度
            EncodedTable[i].length = huffmanCodes[i].size();
            // 将编码转换成字节数据
            for (int j = 0; j < huffmanCodes[i].size(); j += 8) {
                std::string subStr = huffmanCodes[i].substr(j, 8);
                // 不足8位补零
                while (subStr.size() < 8) {
                    subStr += "0";
                }
                unsigned char byte = 0;
                for (char bit : subStr) {
                    byte = byte << 1;
                    byte |= (bit == '1') ? 1 : 0;
                }
                EncodedTable[i].bitCoded.push_back(byte);
            }
        }
        // 输出编码表
        std::string outputEncodingTable = "output/code.txt";
        std::ofstream tableFile(outputEncodingTable);
        if (!tableFile) {
            std::cerr << "Error opening output file: " << outputEncodingTable << std::endl;
            return;
        }
        // 写入原始文本长度
        tableFile << processedContent.size() << std::endl;
        for (auto &entry : EncodedTable) {
            if (entry.length == 0) {
                continue;
            }
            tableFile << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(entry.byteCode);
            tableFile << " 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << entry.length;
            for (unsigned char byte : entry.bitCoded) {
                tableFile << " 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }
            tableFile << std::dec << std::endl;
        }
        tableFile.close();

        // 11. 获取压缩后的数据并计算压缩数据的HASH值
        std::vector<unsigned char> compressedData;
        for (unsigned char c : processedContent) {
            for (unsigned char byte : EncodedTable[c].bitCoded) {
                compressedData.push_back(byte);
            }
        }
        std::string dataHash = Common::calculateHash(compressedData);
        std::cout << "Compressed Data Hash: " << dataHash << std::endl;
        std::cout << "Compressed Data Size: " << compressedData.size() << " bytes" << std::endl;

        // 12. 将压缩数据写入文件
        std::string outputCompressedFile = inputFile + ".huf";
        std::ofstream outFile(outputCompressedFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputCompressedFile << std::endl;
            return;
        }
        // [attention!] `reinterpret_cast<const char *>`将unsigned char*转换成const char*
        outFile.write(reinterpret_cast<const char *>(compressedData.data()), compressedData.size());
        outFile.close();

        // 13. 显示压缩数据后16个字节
        std::cout << "Last 16 Bytes of Compressed Data:" << std::endl;
        int startPos = std::max(0, static_cast<int>(compressedData.size()) - 16);
        for (int i = startPos; i < compressedData.size(); i++) {
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(compressedData[i]) << " ";
        }
        std::cout << std::dec << std::endl;

        // 14. 释放哈夫曼树结点
        deleteTree(huffmanTreeRoot);
    }
}