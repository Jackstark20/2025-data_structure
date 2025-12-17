#pragma once
#include <string>
#include <vector>
#include <cstdint>
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

#ifdef QT_CORE_LIB
#include <QString>
#include <QByteArray>
QString encodeTextQt(const QString &text);
QString decodeTextQt(const QString &encoded_combined);


QByteArray encodeImageQt(const QByteArray &image_data);
QByteArray decodeImageQt(const QByteArray &encoded_combined);
#endif

} // namespace backend_api
