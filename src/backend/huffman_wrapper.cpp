#include "huffman_wrapper.h"

// 这些实现都是轻量的转发：调用原始实现（在 `UIwithPIC.cpp` 中定义），
// 这样可以立刻开始按目录划分代码，后续再把实现移动进来并替换。

extern std::vector<std::pair<wchar_t,int>> getFrequencySorted(const std::wstring& text);
extern std::vector<std::pair<unsigned char,int>> getByteFrequencySorted(const std::vector<unsigned char>& data);

extern std::wstring encodeText(const std::wstring& text, const std::unordered_map<wchar_t, std::wstring>& codeMap);
extern std::wstring encodeImage(const std::vector<unsigned char>& data, const std::unordered_map<unsigned char, std::wstring>& codeMap);

extern std::vector<unsigned char> LoadImageData(const std::wstring& path, int& width, int& height, int& bitsPerPixel);
extern bool SaveImageData(const std::wstring& path, const std::vector<unsigned char>& data, int width, int height, int bitsPerPixel);

namespace backend {

std::vector<CharFreq> getFrequencySorted(const std::wstring &text) {
    auto v = ::getFrequencySorted(text);
    std::vector<CharFreq> out;
    out.reserve(v.size());
    for (auto &p : v) out.emplace_back(p.first, p.second);
    return out;
}

std::vector<ByteFreq> getByteFrequencySorted(const std::vector<uint8_t> &data) {
    auto v = ::getByteFrequencySorted(data);
    std::vector<ByteFreq> out;
    out.reserve(v.size());
    for (auto &p : v) out.emplace_back((uint8_t)p.first, p.second);
    return out;
}

std::wstring encodeText(const std::wstring &text, const std::unordered_map<wchar_t, std::wstring> &codeMap) {
    return ::encodeText(text, codeMap);
}

std::wstring encodeImage(const std::vector<uint8_t> &data, const std::unordered_map<uint8_t, std::wstring> &codeMap) {
    // convert types and call global
    std::unordered_map<unsigned char, std::wstring> tmp;
    for (auto &p : codeMap) tmp[(unsigned char)p.first] = p.second;
    return ::encodeImage(data, tmp);
}

std::vector<uint8_t> LoadImageData(const std::wstring &path, int &width, int &height, int &bitsPerPixel) {
    return ::LoadImageData(path, width, height, bitsPerPixel);
}

bool SaveImageData(const std::wstring &path, const std::vector<uint8_t> &data, int width, int height, int bitsPerPixel) {
    return ::SaveImageData(path, data, width, height, bitsPerPixel);
}

} // namespace backend
