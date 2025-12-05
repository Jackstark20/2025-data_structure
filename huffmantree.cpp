#include "huffmantree.h"
#include <algorithm>

HuffmanBuilder::HuffmanBuilder(const QVector<QPair<QString,int>>& items)
    : m_items(items)
{
}

QVector<HuffStep> HuffmanBuilder::buildSteps()
{
    QVector<HuffStep> steps;
    // create initial nodes
    QVector<QSharedPointer<HuffNode>> pool;
    int nextId = 0;
    for (const auto &p : m_items) {
        auto node = QSharedPointer<HuffNode>::create(p.second, p.first, nextId++);
        pool.append(node);
    }

    // use simple greedy algorithm
    while (pool.size() > 1) {
        // sort ascending by weight, tie-break by id
        std::sort(pool.begin(), pool.end(), [](const QSharedPointer<HuffNode>& n1, const QSharedPointer<HuffNode>& n2){
            if (n1->weight != n2->weight) return n1->weight < n2->weight;
            return n1->id < n2->id;
        });

        auto a = pool.takeFirst();
        auto b = pool.takeFirst();
        auto parent = QSharedPointer<HuffNode>::create(a->weight + b->weight, QString(), nextId++);
        parent->left = a;
        parent->right = b;

        HuffStep s;
        s.a = a;
        s.b = b;
        s.parent = parent;
        steps.append(s);

        pool.append(parent);
    }

    return steps;
}
