#include "common.h"
#include <sstream>

namespace Common {
    // 解密函数
    void encrypt(std::vector<unsigned char> &data, const std::string &key) {
        if (key.empty()) {
            // 偏移量解密
            for (unsigned char &b : data) {
                b += 0x55;
            }
        } else {
            // 异或解密
            int index = 0;
            for (unsigned char &b : data) {
                b ^= key[index];
                index = (index + 1) % key.size();
            }
        }
    }
    void decrypt(std::vector<unsigned char> &data, const std::string &key) {
        if (key.empty()) {
            // 偏移量解密
            for (unsigned char &b : data) {
                b -= 0x55;
            }
        } else {
            // 异或解密
            int index = 0;
            for (unsigned char &b : data) {
                b ^= key[index];
                index = (index + 1) % key.size();
            }
        }
    }
    std::string extractFileName(const std::string &filename) {
        int slash_pos = filename.find_last_of("/\\");
        int dot_pos = filename.find_last_of('.');
        return filename.substr(slash_pos + 1, dot_pos - slash_pos - 1);
    }
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
    std::string byteToBinary(unsigned char byte) {
        std::string binary;
        for (int i = 7; i >= 0; i--) {
            binary += ((byte >> i) & 1) ? '1' : '0';
        }
        return binary;
    }
}