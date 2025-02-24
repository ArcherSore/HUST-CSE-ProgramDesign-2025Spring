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
                      bool encrypt = false,
                      const std::string &key = "") {
        // 1. 读取文本文件的内容
        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening input file: " << inputFile << std::endl;
            return;
        }
        std::vector<unsigned char> content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();
        
        // 2. 插入扩展信息：发送人和接收人信息
        // 格式：发送人+接收人+原文本内容
        std::vector<unsigned char> processedContent;
        if (!senderInfo.empty()) {
            processedContent.insert(processedContent.end(), senderInfo.begin(), senderInfo.end());
            processedContent.push_back('\n');
        }
        if (!receiverInfo.empty()) {
            processedContent.insert(processedContent.end(), receiverInfo.begin(), receiverInfo.end());
            processedContent.push_back('\n');
        }
        processedContent.insert(processedContent.end(), content.begin(), content.end());

        // 3. 写入原文件
        std::ofstream newinputFile(inputFile, std::ios::binary);
        if (!newinputFile) {
            std::cerr << "Error opening output file: " << inputFile << std::endl;
            return;
        }
        newinputFile.write(reinterpret_cast<const char*>(processedContent.data()), processedContent.size());
        newinputFile.close();

        // 4. 加密处理
        if (encrypt) {
            Common::encrypt(processedContent, key);
        }

        // 5. 统计每个字节出现的频率
        std::vector<int> freq(256, 0);
        for (unsigned char c : processedContent) {
            freq[c]++;
        }
        
        // 6. 构造结点数组
        std::vector<Node *> nodes;
        for (int i = 0; i < 256; i++) {
            if (freq[i] == 0) {
                continue;
            }
            nodes.push_back(new Node(static_cast<unsigned char>(i), freq[i]));
        }

        // 7. 使用公共模块的堆排序堆结点数组排序
        auto comp = [](const Node *a, const Node *b) -> bool {
            if (a->freq != b->freq) {
                return a->freq < b->freq;
            }
            return a->byteVal < b->byteVal;
        };
        Common::heapSort(nodes, comp);
        // 显示排序后的词频统计表
        std::cout << "*****Sorted Frequency List*****" << std::endl;
        std::cout << "Byte  Freq" << std::endl;
        for (auto n : nodes) {
            /*
                hex->整数的输出设置为十六进制
                dec->整数的输出设置为十进制
                setw->设置输出字段宽度
                setfill->指定在宽度不足时左侧用字符'0'填充
            */
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(n->byteVal);
            std::cout << '\t' << std::dec << n->freq << std::endl;
        }

        // 8. 用小根堆构建哈夫曼树
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

        // 9. 计算WPL
        int wpl = 0;
        computeWPL(huffmanTreeRoot, 0, wpl);
        std::cout << "********************************" << std::endl;
        std::cout << "Huffman Tree WPL: " << wpl << std::endl;

        // 10. 遍历哈夫曼树得到每个字节对应的编码
        std::vector<std::string> huffmanCodes(256);
        getHuffmanCode(huffmanTreeRoot, "", huffmanCodes);

        // 11. 计算压缩前的数据HASH值
        std::cout << "********************************" << std::endl;
        std::string OriginalDataHash = Common::calculateHash(content);
        std::cout << "Original Data Hash: 0x" << OriginalDataHash << std::endl;
        std::cout << "Original Data Size: " << processedContent.size() << " bytes" << std::endl;

        // 12. 获取编码表并输出到code.txt文件
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
        std::string outputEncodingTable;
        outputEncodingTable = "test/code.txt";
        std::ofstream tableFile(outputEncodingTable);
        if (!tableFile) {
            std::cerr << "Error opening output file: " << outputEncodingTable << std::endl;
            return;
        }
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

        // 13. 获取压缩后的数据并计算压缩数据的HASH值
        // 获取压缩数据
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
        // 如果剩余比特不足8位，则在低位补 0
        if (bitcount > 0) {
            byte <<= (8 - bitcount);
            compressedData.push_back(byte);
        }
        // 输出压缩后数据的HASH值和大小
        std::cout << "********************************" << std::endl;
        std::string CompressedDataHash = Common::calculateHash(compressedData);
        std::cout << "Compressed Data Hash: 0x" << CompressedDataHash << std::endl;
        std::cout << "Compressed Data Size: " << compressedData.size() << " bytes" << std::endl;

        // 14. 将压缩数据写入文件
        std::string outputCompressedFile;
        outputCompressedFile = "test/" + Common::extractFileName(inputFile) + ".hfm";
        std::ofstream outFile(outputCompressedFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputCompressedFile << std::endl;
            return;
        }
        // [attention!] `reinterpret_cast<const char *>`将unsigned char*转换成const char*
        outFile.write(reinterpret_cast<const char *>(compressedData.data()), compressedData.size());
        outFile.close();

        // 15. 显示压缩数据后16个字节
        std::cout << "********************************" << std::endl;
        std::cout << "Last 16 Bytes of Compressed Data:" << std::endl;
        int startPos = std::max(0, static_cast<int>(compressedData.size()) - 16);
        for (int i = startPos; i < compressedData.size(); i++) {
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(compressedData[i]) << " ";
        }
        std::cout << std::dec << std::endl;
        std::cout << "********************************" << std::endl;

        // 16. 释放哈夫曼树结点
        deleteTree(huffmanTreeRoot);
    }
}