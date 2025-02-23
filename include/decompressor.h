#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include <string>

namespace Decompressor {
    void decompressFile(const std::string &inputFile,
                        const std::string &senderInfo,
                        const std::string &receiverInfo,
                        bool encrypt,
                        const std::string &key);
}

#endif // DECOMPRESSOR_H