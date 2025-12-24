#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>  // 添加这个头文件
#include "EncodingUtils.h"

// 外部友好薄封装（UTF-8 / Qt 适配）
namespace backend_api {

// 将 UTF-8 文本编码为一个合并字符串：<code_table>|<bits>
// code_table 与 bits 均以宽字符串序列化后转换为 UTF-8 返回。
::std::string encodeTextUtf8(const ::std::string &utf8_text);

// 从 encodeTextUtf8 返回的字符串解码并返回原始 UTF-8 文本（若失败返回空字符串）
::std::string decodeTextUtf8(const ::std::string &encoded_combined);

// 将图片字节数据编码为一个合并字符串：<code_table>|<bits>
// code_table 与 bits 均以宽字符串序列化后转换为 UTF-8 返回。
::std::string encodeImage(const ::std::vector<uint8_t> &image_data);

// 从 encodeImage 返回的字符串解码并返回原始图片数据（若失败返回空向量）
::std::vector<uint8_t> decodeImage(const ::std::string &encoded_combined);

// 直接从文件编码文本并保存为.huf文件
bool encodeTextFile(const ::std::string &input_file_path, const ::std::string &output_huf_path);

// 直接从.huf文件解码并保存为文本文件
bool decodeTextFile(const ::std::string &input_huf_path, const ::std::string &output_file_path);

// 直接从文件编码图片并保存为.huf文件
bool encodeImageFile(const ::std::string &input_image_path, const ::std::string &output_huf_path);

// 直接从.huf文件解码并保存为图片文件
bool decodeImageFile(const ::std::string &input_huf_path, const ::std::string &output_image_path);

// 流式读取文本文件并统计字符频率（用于进度显示等）
void streamTextFile(const ::std::string &file_path, 
                   const ::std::function<void(const ::std::unordered_map<char32_t, size_t> &)> &callback, 
                   size_t batch_size = 1024);

#ifdef QT_CORE_LIB
#include <QString>
#include <QByteArray>
QString encodeTextQt(const QString &text);
QString decodeTextQt(const QString &encoded_combined);


QByteArray encodeImageQt(const QByteArray &image_data);
QByteArray decodeImageQt(const QByteArray &encoded_combined);
#endif

} // namespace backend_api