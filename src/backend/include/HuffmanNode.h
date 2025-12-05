#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H


#pragma once

#include <windows.h>

struct HuffmanNode {
    union {
        wchar_t ch;  // 文本字符
        BYTE byte;   // 图像字节
    };
    int freq;
    bool isByte;
    HuffmanNode* parent;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(wchar_t c, int f);
    HuffmanNode(BYTE b, int f);
    ~HuffmanNode();
};

#endif // HUFFMAN_NODE_H