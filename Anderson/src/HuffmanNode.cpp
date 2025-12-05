#include "HuffmanNode.h"

HuffmanNode::HuffmanNode(wchar_t c, int f) 
    : ch(c), freq(f), isByte(false), parent(nullptr), left(nullptr), right(nullptr) {}

HuffmanNode::HuffmanNode(BYTE b, int f) 
    : byte(b), freq(f), isByte(true), parent(nullptr), left(nullptr), right(nullptr) {}

HuffmanNode::~HuffmanNode() {
    // 递归删除子节点
}