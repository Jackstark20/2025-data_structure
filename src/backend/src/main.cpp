#include "HuffmanNode.h"
#include "HuffmanTree.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>
#include <codecvt>
typedef unsigned char BYTE;
// 测试文本编码和解码功能
void testTextHuffman() {
    std::wcout << L"=== 测试文本哈夫曼编码/解码 ===" << std::endl;

    // 准备测试文本
    std::wstring testText = L"this is a test string for huffman encoding";
    std::wcout << L"原始文本: " << testText << std::endl;

    // 统计字符频率
    std::unordered_map<wchar_t, int> freqMap;
    for (wchar_t ch : testText) {
        freqMap[ch]++;
    }

    // 转换为频率向量
    std::vector<std::pair<wchar_t, int>> freqVec(freqMap.begin(), freqMap.end());

    // 构建哈夫曼树
    HuffmanTree tree;
    tree.buildForText(freqVec);

    // 获取编码表
    auto codeMap = tree.getCharCodeMap();
    std::wcout << L"编码表: " << std::endl;
    for (const auto& pair : codeMap) {
        std::wcout << L"字符 '" << pair.first << L"' : " << pair.second << std::endl;
    }

    // 编码文本
    std::wstring encoded = tree.encodeText(testText, codeMap);
    std::wcout << L"编码结果长度: " << encoded.size() << L" 位" << std::endl;

    // 解码文本
    std::wstring decoded = tree.decodeText(encoded);
    std::wcout << L"解码文本: " << decoded << std::endl;

    // 验证结果
    if (decoded == testText) {
        std::wcout << L"文本测试: 成功" << std::endl << std::endl;
    } else {
        std::wcout << L"文本测试: 失败" << std::endl << std::endl;
    }

    // 测试编码表序列化/反序列化
    std::wstring serialized = tree.serializeCodes();
    HuffmanTree tree2;
    if (tree2.deserializeCodes(serialized)) {
        std::wstring decoded2 = tree2.decodeText(encoded);
        if (decoded2 == testText) {
            std::wcout << L"编码表序列化测试: 成功" << std::endl << std::endl;
        } else {
            std::wcout << L"编码表序列化测试: 失败" << std::endl << std::endl;
        }
    } else {
        std::wcout << L"编码表反序列化失败" << std::endl << std::endl;
    }
}

// 测试图片字节编码和解码功能
void testImageHuffman() {
    std::wcout << L"=== 测试图片哈夫曼编码/解码 ===" << std::endl;

    // 生成测试图片数据 (随机字节)
    std::vector<BYTE> imageData;
    for (int i = 0; i < 100; ++i) {
        imageData.push_back(rand() % 256);  // 随机字节
    }
    std::wcout << L"原始数据长度: " << imageData.size() << L" 字节" << std::endl;

    // 统计字节频率
    auto freqVec = getByteFrequencySorted(imageData);

    // 构建哈夫曼树
    HuffmanTree tree;
    tree.buildForImage(freqVec);

    // 获取编码表
    auto codeMap = tree.getByteCodeMap();
    std::wcout << L"编码表大小: " << codeMap.size() << L" 个条目" << std::endl;

    // 编码数据
    std::wstring encoded = encodeImage(imageData, codeMap);
    std::wcout << L"编码结果长度: " << encoded.size() << L" 位" << std::endl;

    // 解码数据
    std::vector<BYTE> decoded = tree.decodeImage(encoded);
    std::wcout << L"解码数据长度: " << decoded.size() << L" 字节" << std::endl;

    // 验证结果
    if (decoded == imageData) {
        std::wcout << L"图片测试: 成功" << std::endl << std::endl;
    } else {
        std::wcout << L"图片测试: 失败" << std::endl << std::endl;
    }

    // 测试二进制位流解码
    // 将编码字符串转换为字节数组
    std::vector<uint8_t> bitData((encoded.size() + 7) / 8, 0);
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == L'1') {
            bitData[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    std::vector<BYTE> decodedFromBits = tree.decodeImageFromBits(bitData.data(), encoded.size());
    if (decodedFromBits == imageData) {
        std::wcout << L"二进制位流解码测试: 成功" << std::endl << std::endl;
    } else {
        std::wcout << L"二进制位流解码测试: 失败" << std::endl << std::endl;
    }
}

// 测试文件读取功能
void testFileReading(const std::string& filePath) {
    std::wcout << L"=== 测试文件读取功能 ===" << std::endl;
    try {
        auto charMap = Text_file_read(filePath);
        std::wcout << L"成功读取文件: " << filePath.c_str() << std::endl;
        std::wcout << L"统计到的不同字符数: " << charMap.size() << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::wcout << L"文件读取错误: " << e.what() << std::endl << std::endl;
    }
}

int main() {
    // 设置控制台宽字符输出
    setlocale(LC_ALL, ""); // 使用系统默认编码
    std::wcout.imbue(std::locale(""));

    // 运行文本测试
    testTextHuffman();

    // 运行图片数据测试
    testImageHuffman();

    // 测试文件读取（可替换为实际测试文件路径）
    testFileReading("C:/Users/19807/OneDrive/Desktop/datastructure/2025-data_structure/lhz/test.txt");

    system("pause");
    return 0;
}