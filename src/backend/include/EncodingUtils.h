#ifndef ENCODING_UTILS_H
#define ENCODING_UTILS_H

#include <string>
#include <windows.h>

// 宽字符串转UTF-8字符串（无BOM）
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    //计算宽字符串转换为utf8之后所需字节数（包含NULL终止符）
    int buf_len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    //异常处理，转换失败返回空字符串
    if (buf_len <= 0) return "";
    //创建对应长度的utf8字符串缓冲区
    std::string str(buf_len, 0);
    //将宽字符串转换后的结果写入缓冲区
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], buf_len, NULL, NULL);
    str.pop_back();  // 移除末尾空字符
    return str;
}

// UTF-8字符串转宽字符串
inline std::wstring utf8_to_wstring(const std::string& str) {
    //计算utf8字符串转换为宽字符串之后所需的字节数
    int buf_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    //异常处理，转换失败返回空宽字符串
    if (buf_len <= 0) return L"";
    //创建对应长度的宽字符串缓冲区
    std::wstring wstr(buf_len, 0);
    //将utf8字符串转换后的结果写入缓冲区
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], buf_len);
    //移除末尾空字符
    wstr.pop_back();
    return wstr;
}

#endif // ENCODING_UTILS_H