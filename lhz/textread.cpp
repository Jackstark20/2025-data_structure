#include <stdio.h>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <codecvt>
#include <locale>
#include <fstream>
#include <stdexcept>  // 补充runtime_error所需头文件

std::unordered_map<char32_t, size_t> Text_file_read(const std::string& file_path)
{
    std::unordered_map<char32_t, size_t> char_map;  // 变量名避免与std::map重名

    // 1. 打开文件：二进制模式避免换行符转换
    std::wifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件：" + file_path);
    }

    // 2. 配置UTF-8编码转换
    typedef std::codecvt_utf8<char32_t, 0x10FFFF, std::consume_header> utf8_convert;
    file.imbue(std::locale(file.getloc(), new utf8_convert));

    // 3. 读取字符（修复类型不匹配问题）
    char32_t ch;  
    while (file.get(reinterpret_cast<wchar_t&>(ch))) {  // 类型转换适配wifstream
        // 统计可见字符、换行、制表符
        if (ch >= 0x20 || ch == '\n' || ch == '\t') {
            char_map[ch]++;
        }
    }

    return char_map;
}

int main() {
    try {
        auto result = Text_file_read("./test.txt");
        for (const auto& pair : result) {
    std::cout << "字符：";
    // 处理可打印字符（直接输出）
    if (pair.first >= 0x20 && pair.first <= 0x7e) {
        std::cout << static_cast<char>(pair.first);
    }
    // 处理换行/制表符（特殊标注）
    else if (pair.first == '\n') {
        std::cout << "\\n（换行符）";
    } else if (pair.first == '\t') {
        std::cout << "\\t（制表符）";
    }
    // 其他字符（仍显示Unicode）
    else {
        std::cout << "U+" << std::hex << pair.first;
    }
    std::cout << " 出现次数：" << std::dec << pair.second << std::endl;
}
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }
    return 0;
}