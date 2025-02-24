#include "ui.h"
#include <iostream>
#include <cstdio>
#include <array>
#include <memory>
#include <stdexcept>
#include "compressor.h"
#include "decompressor.h"

std::string executeCommand(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

void UI::showMenu() {
    std::string choice = executeCommand("zenity --list --title=\"Operation Selection\" --text=\"Please select an operation\" --column=\"Operation\" \"Compress File\" \"Decompress File\"");
    
    if (choice == "Compress File") {
        processCompression();
    } else if (choice == "Decompress File") {
        processDecompression();
    } else {
        system("zenity --error --text=\"Invalid selection\"");
    }
}

void UI::processCompression() {
    std::string inputFile = executeCommand("zenity --file-selection --title=\"Select File to Compress\" --filename=test/");
    if (inputFile.empty()) return;

    std::string senderInfo = executeCommand("zenity --entry --title=\"Sender Information\" --text=\"Format: Student ID Name\"");
    std::string receiverInfo = executeCommand("zenity --entry --title=\"Receiver Information\" --text=\"Format: Student ID Name\"");
    
    bool encrypt = system("zenity --question --title=\"Encryption Option\" --text=\"Enable encryption?\" --ok-label=\"Yes\" --cancel-label=\"No\"") == 0;
    
    std::string key;
    if (encrypt) {
        key = executeCommand("zenity --entry --title=\"Encryption Key\" --text=\"Please enter encryption key\" --hide-text");
    }

    Compressor::compressFile(inputFile, senderInfo, receiverInfo, encrypt, key);
    
    system(("zenity --info --text=\"File compressed: " + inputFile + ".compressed\"").c_str());
}

void UI::processDecompression() {
    std::string compressedFile = executeCommand("zenity --file-selection --title=\"Select File to Decompress\" --filename=test/ --file-filter=\"*.compressed\"");
    if (compressedFile.empty()) return;

    std::string senderInfo = executeCommand("zenity --entry --title=\"Sender Information\" --text=\"Format: Student ID Name\"");
    std::string receiverInfo = executeCommand("zenity --entry --title=\"Receiver Information\" --text=\"Format: Student ID Name\"");

    bool decrypt = system("zenity --question --title=\"Decryption Option\" --text=\"Require decryption?\" --ok-label=\"Yes\" --cancel-label=\"No\"") == 0;
    
    std::string key;
    if (decrypt) {
        key = executeCommand("zenity --entry --title=\"Decryption Key\" --text=\"Please enter decryption key\" --hide-text");
    }

    HashDecompressor::decompressFile(compressedFile, senderInfo, receiverInfo, decrypt, key);
    TrieDecompressor::decompressFile(compressedFile, senderInfo, receiverInfo, decrypt, key);
    
    system(("zenity --info --text=\"File decompressed: " + compressedFile + ".decompressed\"").c_str());
}