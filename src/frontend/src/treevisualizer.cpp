#include "treevisualizer.h"
#include <QGraphicsLineItem>

TreeVisualizer::TreeVisualizer(QWidget *parent) : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
}

TreeVisualizer::~TreeVisualizer() {
    delete m_scene;
}

TreeNodeItem* TreeVisualizer::buildNode(HuffmanNode *node) {
    if (!node) return nullptr;
    TreeNodeItem *item = new TreeNodeItem(node);
    m_scene->addItem(item);

    // 递归构建子节点
    TreeNodeItem *left = buildNode(node->left);
    TreeNodeItem *right = buildNode(node->right);
    item->setLeftChild(left);
    item->setRightChild(right);

    // 绘制连线
    if (left) {
        QGraphicsLineItem *line = new QGraphicsLineItem(
            item->pos().x(), item->pos().y() + item->boundingRect().height()/2,
            left->pos().x(), left->pos().y() - left->boundingRect().height()/2
        );
        m_scene->addItem(line);
    }
    if (right) {
        QGraphicsLineItem *line = new QGraphicsLineItem(
            item->pos().x(), item->pos().y() + item->boundingRect().height()/2,
            right->pos().x(), right->pos().y() - right->boundingRect().height()/2
        );
        m_scene->addItem(line);
    }
    return item;
}

void TreeVisualizer::layoutTree(TreeNodeItem *root, int x, int y, int levelSpacing, int nodeSpacing) {
    if (!root) return;
    root->setPos(x, y);

    // 计算子节点位置
    int childY = y + levelSpacing;
    TreeNodeItem *left = root->getLeftChild();
    TreeNodeItem *right = root->getRightChild();

    if (left && right) {
        layoutTree(left, x - nodeSpacing, childY, levelSpacing, nodeSpacing/2);
        layoutTree(right, x + nodeSpacing, childY, levelSpacing, nodeSpacing/2);
    } else if (left) {
        layoutTree(left, x, childY, levelSpacing, nodeSpacing/2);
    } else if (right) {
        layoutTree(right, x, childY, levelSpacing, nodeSpacing/2);
    }
}

void TreeVisualizer::buildFromHuffmanTree(HuffmanTree *tree) {
    m_scene->clear();
    TreeNodeItem *rootItem = buildNode(tree->getRoot());
    if (rootItem) {
        layoutTree(rootItem, width()/2, 50); // 从中心顶部开始布局
        fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    }
}