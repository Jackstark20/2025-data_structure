#ifndef ENCODING_UTILS_H
#define ENCODING_UTILS_H

#include <string>
#include <windows.h>

// 宽字符串转UTF-8字符串（无BOM）
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    int buf_len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (buf_len <= 0) return "";
    std::string str(buf_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], buf_len, NULL, NULL);
    str.pop_back();  // 移除末尾空字符
    return str;
}

// UTF-8字符串转宽字符串
inline std::wstring utf8_to_wstring(const std::string& str) {
    int buf_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (buf_len <= 0) return L"";
    std::wstring wstr(buf_len, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], buf_len);
    wstr.pop_back();
    return wstr;
}

#endif // ENCODING_UTILS_H