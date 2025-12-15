#ifndef TREEVISUALIZER_H
#define TREEVISUALIZER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "treenodeitem.h"
#include "HuffmanTree.h"

class TreeVisualizer : public QGraphicsView {
    Q_OBJECT

public:
    TreeVisualizer(QWidget *parent = nullptr);
    ~TreeVisualizer() override;

    // 从后端哈夫曼树构建可视化
    void buildFromHuffmanTree(HuffmanTree *tree);

private:
    QGraphicsScene *m_scene;
    TreeNodeItem* buildNode(HuffmanNode *node);
    void layoutTree(TreeNodeItem *root, int x = 0, int y = 0, int levelSpacing = 100, int nodeSpacing = 50);
};
#endif // TREEVISUALIZER_H