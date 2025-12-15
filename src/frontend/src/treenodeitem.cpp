#include "treenodeitem.h"
#include <QPainter>
#include <QFontMetrics>
#include "EncodingUtils.h"

TreeNodeItem::TreeNodeItem(HuffmanNode *node, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_node(node), m_leftChild(nullptr), m_rightChild(nullptr) {
    // 构建显示文本
    if (node->isByte) {
        m_displayText = QString("0x%1\n%2").arg(node->byte, 2, 16, QChar('0')).arg(node->freq);
    } else {
        wchar_t ch = node->ch;
        std::string utf8 = wstring_to_utf8(std::wstring(1, ch));
        m_displayText = QString("%1\n%2").arg(QString::fromUtf8(utf8.c_str())).arg(node->freq);
    }
    setFlag(ItemIsMovable);
}

TreeNodeItem::~TreeNodeItem() {}

QRectF TreeNodeItem::boundingRect() const {
    // 修复QFontMetrics使用错误：显式创建QFont对象
    QFont font;
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(m_displayText);
    return textRect.adjusted(-10, -10, 10, 10);  // 10px边距（避免文本截断）
}

void TreeNodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 绘制节点（叶子节点用圆形，非叶子用矩形）
    QRectF rect = boundingRect();
    if (m_node->left == nullptr && m_node->right == nullptr) {
        painter->setBrush(QBrush(Qt::lightGray));
        painter->drawEllipse(rect);
    } else {
        painter->setBrush(QBrush(Qt::white));
        painter->drawRect(rect);
    }

    // 绘制文本
    painter->drawText(rect, Qt::AlignCenter, m_displayText);
}

void TreeNodeItem::setLeftChild(TreeNodeItem *child) {
    m_leftChild = child;
}

void TreeNodeItem::setRightChild(TreeNodeItem *child) {
    m_rightChild = child;
}