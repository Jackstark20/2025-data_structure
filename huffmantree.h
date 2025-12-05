#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <functional>

struct HuffNode {
    int weight;
    QString label; // for leaves
    QSharedPointer<HuffNode> left;
    QSharedPointer<HuffNode> right;
    int id;
    HuffNode(int w = 0, const QString &l = QString(), int _id = -1)
        : weight(w), label(l), left(nullptr), right(nullptr), id(_id) {}
};

struct HuffStep {
    QSharedPointer<HuffNode> a;
    QSharedPointer<HuffNode> b;
    QSharedPointer<HuffNode> parent;
};

class HuffmanBuilder {
public:
    // input: vector of (label, weight)
    explicit HuffmanBuilder(const QVector<QPair<QString,int>>& items);

    // generate steps of combining two smallest nodes until one root remains
    QVector<HuffStep> buildSteps();

private:
    QVector<QPair<QString,int>> m_items;
};

#endif // HUFFMANTREE_H
