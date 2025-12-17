#include "backend_api.h"
#include "HuffmanTree.h"
#include "EncodingUtils.h"
#include <string>
#include <ios>

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
    size_t sep = combined.find(L'|');
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
    size_t sep = combined.find(L'|');
    if (sep == std::wstring::npos) return {};
    std::wstring table = combined.substr(0, sep);
    std::wstring bits = combined.substr(sep + 1);

    HuffmanTree tree;
    if (!tree.deserializeCodes(table)) return {};
    return tree.decodeImage(bits);
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
