#include "ui.h"
#include <iostream>
#include <string>
#include "compressor.h"
#include "decompressor.h"

void UI::showMenu() {
    int choice = 1;
    std::cout << "Please select an operation:" << std::endl;
    std::cout << "1. Compress a file" << std::endl;
    std::cout << "2. Decompress a file" << std::endl;
    std::cout << "Enter your choice (1/2): ";
    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1) {
        processCompression();
    } else if (choice == 2) {
        processDecompression();
    } else {
        std::cout << "Invalid choice. Exiting." << std::endl;
    }
}

void UI::processCompression() {
    std::string inputFile;
  
    // 提示用户输入需要压缩的文本文件名称
    std::cout << "Enter the input text file name to compress: ";
    std::getline(std::cin, inputFile);

    // // 获取发送人和接收人信息（可用于扩展功能）
    std::string senderInfo, receiverInfo;
    std::cout << "Enter sender info: ";
    std::getline(std::cin, senderInfo);
    std::cout << "Enter receiver info: ";
    std::getline(std::cin, receiverInfo);

    // // 询问用户是否需要加密处理
    char encryptChoice;
    bool encrypt = false;
    std::cout << "Do you want to apply encryption? (y/n): ";
    std::cin >> encryptChoice;
    if(encryptChoice == 'y' || encryptChoice == 'Y') {
        encrypt = true;
    }
    std::cin.ignore();

    Compressor::compressFile(inputFile, senderInfo, receiverInfo, encrypt);
}

void UI::processDecompression() {
    std::string compressedFile;
    // 获取压缩文件名称和编码表文件名称
    std::cout << "Enter the compressed file name: ";
    std::getline(std::cin, compressedFile);

    // 判断是否需要解密
    char decryptChoice;
    bool decrypt = false;
    std::cout << "Is the file encrypted? (y/n): ";
    std::cin >> decryptChoice;
    if(decryptChoice == 'y' || decryptChoice == 'Y') {
        decrypt = true;
    }
    std::cin.ignore();

    // 调用 Decompressor 模块的 decompressFile() 函数执行解压操作
    Decompressor::decompressFile(compressedFile, decrypt);
}