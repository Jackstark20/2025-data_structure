#include "backend_api.h"
#include "HuffmanTree.h"
#include "EncodingUtils.h"
#include <string>
#include <ios>
#include <algorithm>

namespace backend_api {

std::string encodeTextUtf8(const std::string &utf8_text) {
    // 转为宽字符串
    std::wstring wtext = utf8_to_wstring(utf8_text);

    // 统计频率
    std::unordered_map<wchar_t, int> freqMap;
    for (wchar_t c : wtext) freqMap[c]++;
    std::vector<std::pair<wchar_t,int>> freqVec(freqMap.begin(), freqMap.end());

    // 构建哈夫曼树并编码
    HuffmanTree tree;
    tree.buildForText(freqVec);
    auto codeMap = tree.getCharCodeMap();
    std::wstring bits = tree.encodeText(wtext, codeMap);

    // 获取序列化编码表
    std::wstring table = tree.getSerializedCodeTable();

    // 合并为 <table>|<bits>
    std::wstring combined = table + L"|" + bits;
    return wstring_to_utf8(combined);
}

std::string decodeTextUtf8(const std::string &encoded_combined) {
    std::wstring combined = utf8_to_wstring(encoded_combined);
    
    // 从后往前查找分隔符，确保编码表部分完整
    size_t sep = std::wstring::npos;
    for (size_t i = combined.size() - 1; i > 0; --i) {
        if (combined[i] == L'|') {
            // 检查后面是否是纯二进制位流
            bool isBits = true;
            for (size_t j = i + 1; j < combined.size(); ++j) {
                if (combined[j] != L'0' && combined[j] != L'1') {
                    isBits = false;
                    break;
                }
            }
            if (isBits) {
                sep = i;
                break;
            }
        }
    }
    
    if (sep == std::wstring::npos) return std::string();
    std::wstring table = combined.substr(0, sep);
    std::wstring bits = combined.substr(sep + 1);

    HuffmanTree tree;
    if (!tree.deserializeCodes(table)) return std::string();
    std::wstring decoded = tree.decodeText(bits);
    return wstring_to_utf8(decoded);
}

std::string encodeImage(const std::vector<uint8_t> &image_data) {
    // 统计频率
    auto freqVec = getByteFrequencySorted(image_data);

    // 构建哈夫曼树并编码
    HuffmanTree tree;
    tree.buildForImage(freqVec);
    auto codeMap = tree.getByteCodeMap();
    std::wstring bits = ::encodeImage(image_data, codeMap);

    // 获取序列化编码表
    std::wstring table = tree.getSerializedCodeTable();

    // 合并为 <table>|<bits>
    std::wstring combined = table + L"|" + bits;
    return wstring_to_utf8(combined);
}

std::vector<uint8_t> decodeImage(const std::string &encoded_combined) {
    std::wstring combined = utf8_to_wstring(encoded_combined);
    
    // 从后往前查找分隔符，确保编码表部分完整
    size_t sep = std::wstring::npos;
    for (size_t i = combined.size() - 1; i > 0; --i) {
        if (combined[i] == L'|') {
            // 检查后面是否是纯二进制位流
            bool isBits = true;
            for (size_t j = i + 1; j < combined.size(); ++j) {
                if (combined[j] != L'0' && combined[j] != L'1') {
                    isBits = false;
                    break;
                }
            }
            if (isBits) {
                sep = i;
                break;
            }
        }
    }
    
    if (sep == std::wstring::npos) return {};
    std::wstring table = combined.substr(0, sep);
    std::wstring bits = combined.substr(sep + 1);

    HuffmanTree tree;
    if (!tree.deserializeCodes(table)) return {};
    return tree.decodeImage(bits);
}

void streamTextFile(const std::string &file_path, 
                   const std::function<void(const std::unordered_map<char32_t, size_t> &)> &callback, 
                   size_t batch_size) {
    // 以二进制模式打开文件
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件：" + file_path);
    }

    // 读取文件全部内容到string（UTF-8格式）
    std::string utf8_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // 手动将UTF-8字符串转char32_t并统计频率
    std::unordered_map<char32_t, size_t> char_map;
    size_t i = 0;
    size_t processed = 0;

    while (i < utf8_str.size()) {
        char32_t ch = 0;
        unsigned char c = (unsigned char)utf8_str[i];

        // UTF-8解码规则
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
            processed++;

            // 每处理batch_size个字符，触发一次回调
            if (processed % batch_size == 0) {
                callback(char_map);
            }
        }
    }

    // 处理剩余的字符
    if (processed > 0) {
        callback(char_map);
    }
}


#ifdef QT_CORE_LIB
QString encodeTextQt(const QString &text) {
    std::string utf8 = text.toUtf8().toStdString();
    std::string enc = encodeTextUtf8(utf8);
    return QString::fromUtf8(enc.c_str());
}

QString decodeTextQt(const QString &encoded_combined) {
    std::string enc = encoded_combined.toUtf8().toStdString();
    std::string dec = decodeTextUtf8(enc);
    return QString::fromUtf8(dec.c_str());
}

QByteArray encodeImageQt(const QByteArray &image_data) {
    std::vector<uint8_t> data(image_data.begin(), image_data.end());
    std::string enc = encodeImage(data);
    return QByteArray::fromStdString(enc);
}

QByteArray decodeImageQt(const QByteArray &encoded_combined) {
    std::string enc = encoded_combined.toStdString();
    std::vector<uint8_t> data = decodeImage(enc);
    return QByteArray(reinterpret_cast<const char*>(data.data()), data.size());
}

#endif

} // namespace backend_api