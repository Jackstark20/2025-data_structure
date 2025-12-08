#include "HuffmanNode.h"
#include "HuffmanTree.h"
#include "EncodingUtils.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <windows.h>
#include <cstdlib>
#include <cstdint>

typedef unsigned char BYTE;

// ========== 文本编码测试（保留原有逻辑） ==========
void testTextHuffman() {
    std::cout << wstring_to_utf8(L"\n=== 测试1：文本哈夫曼编码/解码 ===") << std::endl;

    std::wstring testText = L"this is a test string for huffman encoding";
    std::cout << wstring_to_utf8(L"原始文本: ") << wstring_to_utf8(testText) << std::endl;

    std::unordered_map<wchar_t, int> freqMap;
    for (wchar_t ch : testText) {
        freqMap[ch]++;
    }

    std::vector<std::pair<wchar_t, int>> freqVec(freqMap.begin(), freqMap.end());
    HuffmanTree tree;
    tree.buildForText(freqVec);

    auto codeMap = tree.getCharCodeMap();
    std::cout << wstring_to_utf8(L"编码表（前5个字符）: ") << std::endl;
    int count = 0;
    for (const auto& pair : codeMap) {
        if (count++ >= 5) break; // 只输出前5个，避免刷屏
        std::cout << wstring_to_utf8(L"字符 '") 
                  << wstring_to_utf8(std::wstring(1, pair.first))
                  << wstring_to_utf8(L"' : ") 
                  << wstring_to_utf8(pair.second) << std::endl;
    }

    std::wstring encoded = tree.encodeText(testText, codeMap);
    std::cout << wstring_to_utf8(L"编码结果长度: ") << encoded.size() << wstring_to_utf8(L" 位") << std::endl;

    std::wstring decoded = tree.decodeText(encoded);
    std::cout << wstring_to_utf8(L"解码文本: ") << wstring_to_utf8(decoded) << std::endl;

    if (decoded == testText) {
        std::cout << wstring_to_utf8(L"文本测试: 成功") << std::endl;
    } else {
        std::cout << wstring_to_utf8(L"文本测试: 失败") << std::endl;
    }
}

// ========== 新增：测试Text_file_read（文件读取+字符频率统计） ==========
void testTextFileRead(const std::string& filePath) {
    std::cout << wstring_to_utf8(L"\n=== 测试2：文件读取+字符频率统计 ===") << std::endl;
    try {
        auto charFreqMap = Text_file_read(filePath);
        
        if (charFreqMap.empty()) {
            std::cout << wstring_to_utf8(L"警告：文件为空或无有效字符") << std::endl;
            return;
        }

        std::cout << wstring_to_utf8(L"成功读取文件: ") << filePath << std::endl;
        std::cout << wstring_to_utf8(L"统计到的不同字符数: ") << charFreqMap.size() << std::endl;
        
        std::cout << wstring_to_utf8(L"前10个字符的频率: ") << std::endl;
        int count = 0;
        for (const auto& pair : charFreqMap) {
            if (count++ >= 10) break;
            // 替换原有wcout.imbue，改用wstring_to_utf8输出
            std::cout << wstring_to_utf8(L"字符 '") 
                      << wstring_to_utf8(std::wstring(1, static_cast<wchar_t>(pair.first)))
                      << wstring_to_utf8(L"' : ") << pair.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << wstring_to_utf8(L"文件读取错误: ") << e.what() << std::endl;
    }
}

// ========== 新增：测试getByteFrequencySorted（图片字节频率统计） ==========
void testByteFrequency() {
    std::cout << wstring_to_utf8(L"\n=== 测试3：图片字节频率统计 ===") << std::endl;

    // 生成模拟图片字节数据（100个随机字节）
    std::vector<BYTE> imageData;
    srand(time(0)); // 随机种子
    for (int i = 0; i < 100; ++i) {
        imageData.push_back(rand() % 256);
    }
    std::cout << wstring_to_utf8(L"生成模拟图片字节数: ") << imageData.size() << std::endl;

    // 调用getByteFrequencySorted函数
    auto freqVec = getByteFrequencySorted(imageData);
    
    std::cout << wstring_to_utf8(L"统计到的不同字节数: ") << freqVec.size() << std::endl;
    // 输出前5个字节的频率
    std::cout << wstring_to_utf8(L"前5个字节的频率（字节值: 频次）: ") << std::endl;
    int count = 0;
    for (const auto& pair : freqVec) {
        if (count++ >= 5) break;
        std::cout << (int)pair.first << " : " << pair.second << std::endl;
    }
}

// ========== 完善：测试图片编码解码 ==========
void testImageHuffman() {
    std::cout << wstring_to_utf8(L"\n=== 测试4：图片字节编码/解码 ===") << std::endl;

    // 生成模拟图片数据
    std::vector<BYTE> imageData;
    srand(time(0));
    for (int i = 0; i < 100; ++i) {
        imageData.push_back(rand() % 256);
    }
    std::cout << wstring_to_utf8(L"原始图片数据长度: ") << imageData.size() << wstring_to_utf8(L" 字节") << std::endl;

    // 调用getByteFrequencySorted统计频率
    auto freqVec = getByteFrequencySorted(imageData);

    // 构建哈夫曼树
    HuffmanTree tree;
    tree.buildForImage(freqVec);

    // 获取编码表
    auto codeMap = tree.getByteCodeMap();
    std::cout << wstring_to_utf8(L"图片字节编码表大小: ") << codeMap.size() << std::endl;

    // 编码数据
    std::wstring encoded = encodeImage(imageData, codeMap);
    std::cout << wstring_to_utf8(L"编码结果长度: ") << encoded.size() << wstring_to_utf8(L" 位") << std::endl;

    // 解码数据
    std::vector<BYTE> decoded = tree.decodeImage(encoded);
    std::cout << wstring_to_utf8(L"解码数据长度: ") << decoded.size() << std::endl;

    // 验证结果
    if (decoded == imageData) {
        std::cout << wstring_to_utf8(L"图片编码解码测试: 成功") << std::endl;
    } else {
        std::cout << wstring_to_utf8(L"图片编码解码测试: 失败") << std::endl;
    }

    // 测试二进制位流解码
    std::vector<uint8_t> bitData((encoded.size() + 7) / 8, 0);
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == L'1') {
            bitData[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    std::vector<BYTE> decodedFromBits = tree.decodeImageFromBits(bitData.data(), encoded.size());
    if (decodedFromBits == imageData) {
        std::cout << wstring_to_utf8(L"二进制位流解码测试: 成功") << std::endl;
    } else {
        std::cout << wstring_to_utf8(L"二进制位流解码测试: 失败") << std::endl;
    }
}

// ========== 最终版main函数（全功能测试） ==========
int main() {
    // 1. 强制终端UTF-8编码
    system("chcp 65001 > nul");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 2. 标题
    std::cout << "========================================" << std::endl;
    std::cout << "          哈夫曼编码全功能测试程序      " << std::endl;
    std::cout << "========================================" << std::endl;

    // 3. 依次运行所有测试
    testTextHuffman(); // 测试1：文本编码解码
    
    // 测试2：文件读取（替换为你本地的txt文件路径！）
    std::string testFilePath = "D:\\mycode\\HuffmanProject1\\2025-data_structure\\lhz\\test.txt";
    testTextFileRead(testFilePath);
    
    testByteFrequency(); // 测试3：字节频率统计
    testImageHuffman();  // 测试4：图片编码解码

    // 4. 结束提示
    std::cout << wstring_to_utf8(L"\n========================================") << std::endl;
    std::cout << wstring_to_utf8(L"          所有测试执行完毕              ") << std::endl;
    std::cout << wstring_to_utf8(L"========================================") << std::endl;

    system("pause");
    return 0;
}