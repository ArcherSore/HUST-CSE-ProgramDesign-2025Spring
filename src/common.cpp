#include "common.h"
#include <sstream>

namespace Common {
    // 函数: encrypt
    // 用途: 对数据进行加密，可采用两种方式：
    //       (1) 若密钥为空，则使用偏移量加密：每个字节加上固定值 0x55
    //       (2) 若密钥非空，则使用异或加密，每个字节与密钥中对应字符异或
    //
    // 参数:
//    data - 待加密数据的字节数组
//    key  - 加密密钥（如果为空则使用偏移加密）
    void encrypt(std::vector<unsigned char> &data, const std::string &key) {
        if (key.empty()) {
            // 用偏移量加密
            for (unsigned char &b : data) {
                b += 0x55;
            }
        } else {
            // 用异或法加密
            int index = 0;
            for (unsigned char &b : data) {
                b ^= key[index];
                index = (index + 1) % key.size();
            }
        }
    }
    
    // 函数: decrypt
    // 用途: 对数据进行解密，解密方法与加密时相同
    //
    // 参数:
//    data - 待解密数据的字节数组
//    key  - 解密密钥（如果为空则使用偏移解密）
    void decrypt(std::vector<unsigned char> &data, const std::string &key) {
        if (key.empty()) {
            // 用偏移量解密
            for (unsigned char &b : data) {
                b -= 0x55;
            }
        } else {
            // 用异或法解密（异或本身可逆）
            int index = 0;
            for (unsigned char &b : data) {
                b ^= key[index];
                index = (index + 1) % key.size();
            }
        }
    }
    
    // 函数: extractFileName
    // 用途: 从完整的文件路径中提取文件的基本名称（不包含路径和扩展名）
    //
    // 参数:
//    filename - 文件的完整路径
//
// 返回:
//    文件基础名称，如 "example"（不带扩展名）
    std::string extractFileName(const std::string &filename) {
        int slash_pos = filename.find_last_of("/\\");
        int dot_pos = filename.find_last_of('.');
        return filename.substr(slash_pos + 1, dot_pos - slash_pos - 1);
    }
    
    // 函数: hexToBinary
    // 用途: 将十六进制字符串转换为对应的二进制字符串（每个十六进制字符转换为4位二进制）
    //
    // 参数:
//    hex - 十六进制字符串（应只包含 0-9 和 A-F 大写）
//
// 返回:
//    对应的二进制字符串
    std::string hexToBinary(const std::string &hex) {
        const std::string HEX_BIN[] = {
            "0000", "0001", "0010", "0011",
            "0100", "0101", "0110", "0111",
            "1000", "1001", "1010", "1011",
            "1100", "1101", "1110", "1111"
        };

        std::string binary;
        for (char c : hex) {
            if ('0' <= c && c <= '9') {
                binary += HEX_BIN[c - '0'];
            } else {
                binary += HEX_BIN[c - 'A' + 10];
            }
        }
        return binary;
    }
    
    // 函数: byteToBinary
    // 用途: 将单个字节转换为其8位二进制表示的字符串
    //
    // 参数:
//    byte - 待转换的字节
//
// 返回:
//    8位二进制字符串，如 "01010101"
    std::string byteToBinary(unsigned char byte) {
        std::string binary;
        for (int i = 7; i >= 0; i--) {
            binary += ((byte >> i) & 1) ? '1' : '0';
        }
        return binary;
    }
}