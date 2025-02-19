#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include <string>

namespace Decompressor {
    void decompressFile(const std::string& compressedFile, bool decrypt = false);
}

#endif // DECOMPRESSOR_H