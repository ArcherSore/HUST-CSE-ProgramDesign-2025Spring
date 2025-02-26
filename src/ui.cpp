#include "ui.h"
#include <iostream>
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include "compressor.h"
#include "decompressor.h"

// 函数: executeCommand
// 用途: 执行系统命令并捕获命令行输出，返回输出的字符串
//
// 参数:
//    cmd - 要执行的系统命令字符串
//
// 返回:
//    std::string - 命令执行后的标准输出结果（尾部换行已去除）
//
// 说明:
//    该函数使用 popen() 创建一个管道读取系统命令的输出，并用 unique_ptr 自动管理管道资源。
std::string executeCommand(const char* cmd) {
    std::array<char, 128> buffer;  // 用于存储每次读取的数据
    std::string result;            // 存储完整的命令输出
    // 使用 unique_ptr 自动释放管道资源（pclose）
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    // 不断读取管道输出，直到结束
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    // 移除尾部可能多余的换行符
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

// UI 类成员函数: showMenu
// 用途: 显示 Zenity 图形界面菜单，供用户选择操作（压缩或解压文件）
//
// 说明:
//    使用 Zenity 工具显示对话框，获取用户选择，并根据选择调用具体的处理函数
void UI::showMenu() {
    std::string choice = executeCommand("zenity --list --title=\"Operation Selection\" --text=\"Please select an operation\" --column=\"Operation\" \"Compress File\" \"Decompress File\"");
    
    if (choice == "Compress File") {
        processCompression();
    } else if (choice == "Decompress File") {
        processDecompression();
    } else {
        // 如果选择无效，则显示错误对话框
        system("zenity --error --text=\"Invalid selection\"");
    }
}

// UI 类成员函数: processCompression
// 用途: 处理文件压缩的完整流程，包括文件选择、信息输入、加密选项及调用 Compressor 进行压缩
void UI::processCompression() {
    // 通过 Zenity 选择需要压缩的文件（默认目录：test/）
    std::string inputFile = executeCommand("zenity --file-selection --title=\"Select File to Compress\" --filename=test/");
    if (inputFile.empty()) return; // 若未选择文件，则直接返回

    // 输入发送者信息（格式：学号+姓名）
    std::string senderInfo = executeCommand("zenity --entry --title=\"Sender Information\" --text=\"Format: Student ID Name\"");
    // 输入接收者信息
    std::string receiverInfo = executeCommand("zenity --entry --title=\"Receiver Information\" --text=\"Format: Student ID Name\"");
    
    // 询问是否启用加密（返回值为 0 表示用户点击 Yes）
    bool encrypt = system("zenity --question --title=\"Encryption Option\" --text=\"Enable encryption?\" --ok-label=\"Yes\" --cancel-label=\"No\"") == 0;
    
    std::string key;
    if (encrypt) {
        // 若加密，则需输入加密密钥（密码输入隐藏）
        key = executeCommand("zenity --entry --title=\"Encryption Key\" --text=\"Please enter encryption key\" --hide-text");
    }

    // 调用 Compressor 进行文件压缩处理，传入必要的参数
    Compressor::compressFile(inputFile, senderInfo, receiverInfo, encrypt, key);
    
    // 压缩完成后通过 Zenity 显示提示信息，告知压缩后的文件位置
    system(("zenity --info --text=\"File compressed: " + inputFile + ".compressed\"").c_str());
}

// UI 类成员函数: processDecompression
// 用途: 处理文件解压缩流程，包括压缩文件选择、信息输入、解密选项及调用 Decompressor 进行解压
void UI::processDecompression() {
    // 选择待解压的压缩文件（默认目录为 test/，过滤 .compressed 文件）
    std::string compressedFile = executeCommand("zenity --file-selection --title=\"Select File to Decompress\" --filename=test/ --file-filter=\"*.hfm\"");
    if (compressedFile.empty()) return;

    // 输入发送者与接收者信息，用于后续校验
    std::string senderInfo = executeCommand("zenity --entry --title=\"Sender Information\" --text=\"Format: Student ID Name\"");
    std::string receiverInfo = executeCommand("zenity --entry --title=\"Receiver Information\" --text=\"Format: Student ID Name\"");

    // 询问是否需要解密（返回值 0 表示 Yes）
    bool decrypt = system("zenity --question --title=\"Decryption Option\" --text=\"Require decryption?\" --ok-label=\"Yes\" --cancel-label=\"No\"") == 0;
    
    std::string key;
    if (decrypt) {
        // 若需要解密，则输入解密密钥（密码输入隐藏）
        key = executeCommand("zenity --entry --title=\"Decryption Key\" --text=\"Please enter decryption key\" --hide-text");
    }

    // 调用两种不同的解压缩函数：
    //  HashDecompressor 使用哈希表方式解码，TrieDecompressor 使用字典树解码
    HashDecompressor::decompressFile(compressedFile, senderInfo, receiverInfo, decrypt, key);
    TrieDecompressor::decompressFile(compressedFile, senderInfo, receiverInfo, decrypt, key);
    
    // 解压完成后显示提示信息
    system(("zenity --info --text=\"File decompressed: " + compressedFile + ".decompressed\"").c_str());
}