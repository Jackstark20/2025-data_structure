#include "HuffmanTree.h"
#include <algorithm>
#include <stdio.h>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <codecvt>
#include <locale>
#include <fstream>
#include <stdexcept> 
//流式读取text文件并统计频率
// 替换原有Text_file_read函数的全部内容
std::unordered_map<char32_t, size_t> Text_file_read(const std::string& file_path)
{
    std::unordered_map<char32_t, size_t> char_map;

    // 1. 以二进制模式打开文件（避免换行符转换）
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件：" + file_path);
    }

    // 2. 读取文件全部内容到string（UTF-8格式）
    std::string utf8_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 3. 手动将UTF-8字符串转char32_t（避开locale）
    size_t i = 0;
    while (i < utf8_str.size()) {
        char32_t ch = 0;
        unsigned char c = (unsigned char)utf8_str[i];

        // UTF-8解码规则（兼容单/多字节字符）
        if (c < 0x80) { // 单字节（ASCII）
            ch = c;
            i++;
        } else if (c < 0xE0) { // 双字节
            ch = ((c & 0x1F) << 6) | (utf8_str[i+1] & 0x3F);
            i += 2;
        } else if (c < 0xF0) { // 三字节
            ch = ((c & 0x0F) << 12) | ((utf8_str[i+1] & 0x3F) << 6) | (utf8_str[i+2] & 0x3F);
            i += 3;
        } else if (c < 0xF8) { // 四字节
            ch = ((c & 0x07) << 18) | ((utf8_str[i+1] & 0x3F) << 12) | ((utf8_str[i+2] & 0x3F) << 6) | (utf8_str[i+3] & 0x3F);
            i += 4;
        } else { // 无效UTF-8，跳过
            i++;
            continue;
        }

        // 统计可见字符、换行、制表符
        if (ch >= 0x20 || ch == '\n' || ch == '\t') {
            char_map[ch]++;
        }
    }

    return char_map;
}
//图片字节频率统计
vector<pair<BYTE, int>> getByteFrequencySorted(const vector<BYTE>& data) {
    unordered_map<BYTE, int> freqMap;
    for (BYTE b : data) {
        freqMap[b]++;
    }

    vector<pair<BYTE, int>> freqVec(freqMap.begin(), freqMap.end());

    sort(freqVec.begin(), freqVec.end(), 
        [](const pair<BYTE, int>& a, const pair<BYTE, int>& b) {
            if (a.second != b.second) {
                return a.second < b.second;
            } else {
                return a.first < b.first;
            }
        });

    return freqVec;
}

// 构造函数和析构函数
HuffmanTree::HuffmanTree() : root(nullptr), isImageTree(false) {}

HuffmanTree::~HuffmanTree() {
    destroyNode(root);
    root = nullptr;
    leafnodes.clear();
    charToCode.clear();
    codeToChar.clear();
    byteToCode.clear();
    codeToByte.clear();
}

// 私有方法实现
void HuffmanTree::destroyNode(HuffmanNode* node) {
    if (node) {
        destroyNode(node->left);
        destroyNode(node->right);
        delete node;
    }
}

// 构建文本哈夫曼树
void HuffmanTree::buildForText(const std::vector<std::pair<wchar_t, int>>& freqVec) {
   destroyNode(root);
        root = nullptr;
        leafnodes.clear();
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        isImageTree = false;

        vector<HuffmanNode*> nodes;
        for (const auto& p : freqVec) {
            HuffmanNode* newNode = new HuffmanNode(p.first, p.second);
            nodes.push_back(newNode);
            leafnodes.push_back(newNode);
            charToCode[p.first] = L"";
        }

        while (nodes.size() > 1) {
            sort(nodes.begin(), nodes.end(), [](HuffmanNode* a, HuffmanNode* b) {
                if (a->freq != b->freq) {
                    return a->freq < b->freq;
                } else {
                    bool aIsLeaf = (a->left == nullptr && a->right == nullptr);
                    bool bIsLeaf = (b->left == nullptr && b->right == nullptr);
                    if (aIsLeaf && !bIsLeaf) return true;
                    if (!aIsLeaf && bIsLeaf) return false;
                    int aVal = a->isByte ? (int)a->byte : (int)a->ch;
                    int bVal = b->isByte ? (int)b->byte : (int)b->ch;
                    return aVal < bVal;
                }
            });
            HuffmanNode* left = nodes[0];
            HuffmanNode* right = nodes[1];
            HuffmanNode* parent = new HuffmanNode(L'\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            left->parent = parent;
            
            right->parent = parent;
            nodes.erase(nodes.begin());
            nodes.erase(nodes.begin());
            nodes.push_back(parent);
        }
        if (!nodes.empty()) {
            root = nodes[0];
        }

        for (size_t i = 0; i < leafnodes.size(); ++i) {
            HuffmanNode* current = leafnodes[i];
            wstring code = L"";
            while (current->parent != nullptr) {
                if (current == current->parent->left) {
                    code = L"0" + code;
                } else {
                    code = L"1" + code;
                }
                current = current->parent;
            }
            // 文本树：填充字符->编码 和 编码->字符 映射
            wchar_t ch = leafnodes[i]->ch;
            charToCode[ch] = code;
            codeToChar[code] = ch;
        }
    }
   

// 构建图片哈夫曼树
void HuffmanTree::buildForImage(const std::vector<std::pair<BYTE, int>>& freqVec) {
    destroyNode(root);
        root = nullptr;
        leafnodes.clear();
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        isImageTree = true;

        vector<HuffmanNode*> nodes;
        for (const auto& p : freqVec) {
            HuffmanNode* newNode = new HuffmanNode(p.first, p.second);
            nodes.push_back(newNode);
            leafnodes.push_back(newNode);
            byteToCode[p.first] = L"";
        }

        while (nodes.size() > 1) {
            sort(nodes.begin(), nodes.end(), [](HuffmanNode* a, HuffmanNode* b) {
                if (a->freq != b->freq) {
                    return a->freq < b->freq;
                } else {
                    bool aIsLeaf = (a->left == nullptr && a->right == nullptr);
                    bool bIsLeaf = (b->left == nullptr && b->right == nullptr);
                    if (aIsLeaf && !bIsLeaf) return true;
                    if (!aIsLeaf && bIsLeaf) return false;
                    int aVal = a->isByte ? (int)a->byte : (int)a->ch;
                    int bVal = b->isByte ? (int)b->byte : (int)b->ch;
                    return aVal < bVal;
                }
            });
            HuffmanNode* left = nodes[0];
            HuffmanNode* right = nodes[1];
            HuffmanNode* parent = new HuffmanNode(L'\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            left->parent = parent;
            right->parent = parent;
            nodes.erase(nodes.begin());
            nodes.erase(nodes.begin());
            nodes.push_back(parent);
        }
        if (!nodes.empty()) {
            root = nodes[0];
        }

        for (size_t i = 0; i < leafnodes.size(); ++i) {
            HuffmanNode* current = leafnodes[i];
            wstring code = L"";
            while (current->parent != nullptr) {
                if (current == current->parent->left) {
                    code = L"0" + code;
                } else {
                    code = L"1" + code;
                }
                current = current->parent;
            }
            byteToCode[leafnodes[i]->byte] = code;
            codeToByte[code] = leafnodes[i]->byte;
        }
}

// 获取编码映射表
std::unordered_map<wchar_t, std::wstring> HuffmanTree::getCharCodeMap() const {
    return charToCode;
}

std::unordered_map<BYTE, std::wstring> HuffmanTree::getByteCodeMap() const {
    return byteToCode;
}

// 解码方法
std::wstring HuffmanTree::decodeText(const std::wstring& code) const {
    if (codeToChar.empty()) return L"";
        
        wstring result = L"";
        wstring currentCode;
        for (wchar_t bit : code) {
            currentCode += bit;
            auto it = codeToChar.find(currentCode);
            if (it != codeToChar.end()) {
                result += it->second;
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) {
            return L"解码错误：存在无效编码";
        }
        return result;
}

std::vector<BYTE> HuffmanTree::decodeImage(const std::wstring& code) const {
    if (codeToByte.empty()) return {};
        
        vector<BYTE> result;
        wstring currentCode;
        for (wchar_t bit : code) {
            currentCode += bit;
            auto it = codeToByte.find(currentCode);
            if (it != codeToByte.end()) {
                result.push_back(it->second);
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) {
            return {};  // 解码失败
        }
        return result;
}

std::vector<BYTE> HuffmanTree::decodeImageFromBits(const uint8_t* bytes, uint64_t bitCount) const {
    if (codeToByte.empty()) return {};
        vector<BYTE> result;
        wstring currentCode;
        for (uint64_t i = 0; i < bitCount; ++i) {
            uint64_t byteIndex = i / 8;
            int bitIndex = 7 - (int)(i % 8);
            int bit = (bytes[byteIndex] >> bitIndex) & 1;
            currentCode.push_back(bit ? L'1' : L'0');
            auto it = codeToByte.find(currentCode);
            if (it != codeToByte.end()) {
                result.push_back(it->second);
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) return {};
        return result;
}

// 编码方法
std::wstring HuffmanTree::encodeText(const std::wstring& text, const std::unordered_map<wchar_t, std::wstring>& codeMap) {
    std::wstring encoded;
    for (wchar_t ch : text) {
        auto it = codeMap.find(ch);
        if (it != codeMap.end()) {
            encoded += it->second;
        }
    }
    return encoded;
}

// 新增：编码图片字节数据
std::wstring encodeImage(const std::vector<BYTE>& data, const std::unordered_map<BYTE, std::wstring>& codeMap) {
    wstring encoded;
    for (BYTE b : data) {
        auto it = codeMap.find(b);
        if (it != codeMap.end()) {
            encoded += it->second;
        } else {
            return L"";
        }
    }
    return encoded;
}


// 序列化方法
std::wstring HuffmanTree::serializeCodes() const {
    wstringstream ss;
        ss << (isImageTree ? L"IMAGE" : L"TEXT") << L"|";  // 标记类型
        
        if (isImageTree) {
            for (const auto& p : byteToCode) {
                ss << (int)p.first << L"|" << p.second << L"|";
            }
        } else {
            for (const auto& p : charToCode) {
                ss << (int)p.first << L"|" << p.second << L"|";
            }
        }
        return ss.str();
}

std::wstring HuffmanTree::serializeTextCodes() const {
    wstringstream ss;
        for (const auto& p : charToCode) {
            ss << (int)p.first << L"|" << p.second << L"|";
        }
        return ss.str();
}

// 反序列化方法
bool HuffmanTree::deserializeTextCodes(const std::wstring& data) {
    charToCode.clear();
        codeToChar.clear();
        wstringstream ss(data);
        wstring part;
        while (getline(ss, part, L'|')) {
            if (part.empty()) continue;
            int charCode;
            try {
                charCode = stoi(part);
            } catch (...) {
                return false;
            }
            if (!getline(ss, part, L'|')) return false;
            wchar_t ch = (wchar_t)charCode;
            charToCode[ch] = part;
            codeToChar[part] = ch;
        }
        return !charToCode.empty();
}

bool HuffmanTree::deserializeCodes(const std::wstring& data) {
    charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        
        wstringstream ss(data);
        wstring type;
        if (!getline(ss, type, L'|')) return false;
        
        isImageTree = (type == L"IMAGE");
        wstring part;
        
        while (getline(ss, part, L'|')) {
            if (part.empty()) continue;
            int val;
            try {
                val = stoi(part);
            } catch (...) {
                return false;
            }
            if (!getline(ss, part, L'|')) return false;
            
            if (isImageTree) {
                BYTE b = (BYTE)val;
                byteToCode[b] = part;
                codeToByte[part] = b;
            } else {
                wchar_t ch = (wchar_t)val;
                charToCode[ch] = part;
                codeToChar[part] = ch;
            }
        }
        return isImageTree ? !byteToCode.empty() : !charToCode.empty();
}

// 状态检查
bool HuffmanTree::isImage() const {
    return isImageTree;
}