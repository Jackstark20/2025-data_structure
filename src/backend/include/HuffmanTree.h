#pragma once
#include "HuffmanNode.h"
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>        
#include <cstdint>       
#include <windows.h> 


using namespace std;
std::unordered_map<char32_t, size_t> Text_file_read(const std::string& file_path);//text文件流式读取并统计频率
vector<pair<BYTE, int>> getByteFrequencySorted(const vector<BYTE>& data);//图片字节频率统计
std::wstring encodeImage(const std::vector<BYTE>& data, const std::unordered_map<BYTE, std::wstring>& codeMap);
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
     // 后端内部维护的核心数据（编码时生成）
    std::wstring m_codeTableW;
    std::vector<uint8_t> m_imageBits;
    int m_imgWidth, m_imgHeight, m_imgBpp;
    uint64_t m_imageBitCount;
public:
    HuffmanTree();
    ~HuffmanTree();

    // 构建哈夫曼树
    void buildForText(const std::vector<std::pair<wchar_t, int>>& freqVec);
    void buildForImage(const std::vector<std::pair<BYTE, int>>& freqVec);

    // 1. 获取序列化后的编码表（宽字符版）
    std::wstring getSerializedCodeTable() const { return m_codeTableW; }
    
    // 2. 获取图片编码后的二进制位流
    const std::vector<uint8_t>& getImageBits() const { return m_imageBits; }
    
    // 3. 获取图片元数据（宽、高、位深、总位数）
    void getImageMeta(int& width, int& height, int& bpp, uint64_t& bitCount) const {
        width = m_imgWidth;
        height = m_imgHeight;
        bpp = m_imgBpp;
        bitCount = m_imageBitCount;
    }

    // 编码/解码
    std::unordered_map<wchar_t, std::wstring> getCharCodeMap() const;
    std::unordered_map<BYTE, std::wstring> getByteCodeMap() const;
    std::wstring decodeText(const std::wstring& code) const;
    std::vector<BYTE> decodeImage(const std::wstring& code) const;
    std::vector<BYTE> decodeImageFromBits(const uint8_t* bytes, uint64_t bitCount) const;
    std::wstring encodeText(const std::wstring& text, const std::unordered_map<wchar_t, std::wstring>& codeMap);
    
    // ...existing code...

    // 序列化/反序列化
    std::wstring serializeCodes() const;
    std::wstring serializeTextCodes() const;
    bool deserializeTextCodes(const std::wstring& data);
    bool deserializeCodes(const std::wstring& data);

    // 获取根节点
    HuffmanNode* getRoot() const { return root; }

    // 状态检查
    bool isImage() const;
};