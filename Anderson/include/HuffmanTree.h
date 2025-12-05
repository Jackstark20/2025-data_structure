#pragma once
#include "HuffmanNode.h"
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>        
#include <cstdint>       
#include <windows.h> 

using namespace std;

class HuffmanTree {
private:
    HuffmanNode* root;
    std::unordered_map<wchar_t, std::wstring> charToCode;  // 字符到编码的映射（文本）
    std::unordered_map<std::wstring, wchar_t> codeToChar;   // 编码到字符的映射（文本）
    std::unordered_map<BYTE, std::wstring> byteToCode;      // 字节到编码的映射（图片）
    std::unordered_map<std::wstring, BYTE> codeToByte;     // 编码到字节的映射（图片）
    std::vector<HuffmanNode*> leafnodes;
    bool isImageTree;  // 标记当前树是用于图片还是文本

    void destroyNode(HuffmanNode* node);

public:
    HuffmanTree();
    ~HuffmanTree();

    // 构建哈夫曼树
    void buildForText(const std::vector<std::pair<wchar_t, int>>& freqVec);
    void buildForImage(const std::vector<std::pair<BYTE, int>>& freqVec);

    // 编码/解码
    std::unordered_map<wchar_t, std::wstring> getCharCodeMap() const;
    std::unordered_map<BYTE, std::wstring> getByteCodeMap() const;
    std::wstring decodeText(const std::wstring& code) const;
    std::vector<BYTE> decodeImage(const std::wstring& code) const;
    std::vector<BYTE> decodeImageFromBits(const uint8_t* bytes, uint64_t bitCount) const;
    std::wstring encodeText(const std::wstring& text, const std::unordered_map<wchar_t, std::wstring>& codeMap);
    std::wstring encodeImage(const std::vector<BYTE>& data, const std::unordered_map<BYTE, std::wstring>& codeMap);
    // ...existing code...

    // 序列化/反序列化
    std::wstring serializeCodes() const;
    std::wstring serializeTextCodes() const;
    bool deserializeTextCodes(const std::wstring& data);
    bool deserializeCodes(const std::wstring& data);

    // 状态检查
    bool isImage() const;
};