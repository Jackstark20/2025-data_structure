#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include "EncodingUtils.h"

// 外部友好薄封装（UTF-8 / Qt 适配）
namespace backend_api {

// 将 UTF-8 文本编码为一个合并字符串：<code_table>|<bits>
// code_table 与 bits 均以宽字符串序列化后转换为 UTF-8 返回。
std::string encodeTextUtf8(const std::string &utf8_text);

// 从 encodeTextUtf8 返回的字符串解码并返回原始 UTF-8 文本（若失败返回空字符串）
std::string decodeTextUtf8(const std::string &encoded_combined);

// 将图片字节数据编码为一个合并字符串：<code_table>|<bits>
// code_table 与 bits 均以宽字符串序列化后转换为 UTF-8 返回。
std::string encodeImage(const std::vector<uint8_t> &image_data);

// 从 encodeImage 返回的字符串解码并返回原始图片数据（若失败返回空向量）
std::vector<uint8_t> decodeImage(const std::string &encoded_combined);

// 流式读取文本文件并统计字符频率，支持实时回调更新
// callback: 每读取一定数量字符后调用的回调函数，参数为当前字符频率统计结果（字符 -> 出现次数）
// batch_size: 每读取多少字符触发一次回调（默认1000）
void streamTextFile(const std::string &file_path, 
                   const std::function<void(const std::unordered_map<char32_t, size_t> &)> &callback, 
                   size_t batch_size = 1000);

#ifdef QT_CORE_LIB
#include <QString>
#include <QByteArray>
QString encodeTextQt(const QString &text);
QString decodeTextQt(const QString &encoded_combined);


QByteArray encodeImageQt(const QByteArray &image_data);
QByteArray decodeImageQt(const QByteArray &encoded_combined);
#endif

} // namespace backend_api
