#include "common.h"
#include <sstream>

namespace {
    constexpr uint64_t FNV1A_64_INIT = 0xcbf29ce484222325ULL;
    constexpr uint64_t FNV1A_64_PRIME = 0x100000001b3ULL;

    uint64_t fnv1a_64(const std::string &data) {
        uint64_t hash = FNV1A_64_INIT;
        for (unsigned char byte : data) {
            hash ^= byte;
            hash *= FNV1A_64_PRIME;
        }
        return hash;
    }
}

namespace Common {
    std::string calculateHash(const std::string &data) {
        uint64_t hashValue = fnv1a_64(data);
        std::stringstream ss;
        // 将HASH值转换为16进制字符串
        ss << std::hex << hashValue;
        return ss.str();
    }
    unsigned char encrypt(unsigned char b) {
        // 加密：每个字节加0x55
        return b + 0x55;
    }
    unsigned char decrypt(unsigned char b) {
        // 解密：每个字节减0x55
        return b - 0x55;
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