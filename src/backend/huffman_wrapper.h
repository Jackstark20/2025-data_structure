#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cstdint>
#include <unordered_map>

// 轻量封装：调用现有全局函数（保留在 UIwithPIC.cpp 中）的接口，
// 以便后续把实现逐步移入此目录而不破坏现有主程序。

namespace backend {

using CharFreq = std::pair<wchar_t,int>;
using ByteFreq = std::pair<uint8_t,int>;

// 直接调用现有的频率统计函数
std::vector<CharFreq> getFrequencySorted(const std::wstring &text);
std::vector<ByteFreq> getByteFrequencySorted(const std::vector<uint8_t> &data);

// 调用现有的编码/解码辅助（封装现有全局函数）
std::wstring encodeText(const std::wstring &text, const std::unordered_map<wchar_t, std::wstring> &codeMap);
std::wstring encodeImage(const std::vector<uint8_t> &data, const std::unordered_map<uint8_t, std::wstring> &codeMap);

// 图片 I/O
std::vector<uint8_t> LoadImageData(const std::wstring &path, int &width, int &height, int &bitsPerPixel);
bool SaveImageData(const std::wstring &path, const std::vector<uint8_t> &data, int width, int height, int bitsPerPixel);

} // namespace backend
