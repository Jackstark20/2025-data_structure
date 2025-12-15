#ifndef TREENODEITEM_H
#define TREENODEITEM_H

#include <QGraphicsItem>
#include "HuffmanNode.h"

class TreeNodeItem : public QGraphicsItem {
public:
    TreeNodeItem(HuffmanNode *node, QGraphicsItem *parent = nullptr);
    ~TreeNodeItem() override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    HuffmanNode* getNode() const { return m_node; }
    void setLeftChild(TreeNodeItem *child);
    void setRightChild(TreeNodeItem *child);
    TreeNodeItem* getLeftChild() const { return m_leftChild; }
    TreeNodeItem* getRightChild() const { return m_rightChild; }

private:
    HuffmanNode *m_node;
    TreeNodeItem *m_leftChild;
    TreeNodeItem *m_rightChild;
    QString m_displayText; // 节点显示文本（字符/字节值+频率）
};
#endif // TREENODEITEM_H