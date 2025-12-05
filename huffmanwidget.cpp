#include "huffmanwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QVariantAnimation>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QLinearGradient>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QLabel>
#include <QSlider>
#include <QWheelEvent>

// Simple QGraphicsObject-based node could be used for smoother animations;
// for brevity use QGraphicsItemGroup and animate via pos property on the group (works because group inherits QGraphicsItem)

// Small QGraphicsView subclass to enable smooth wheel zooming
class ZoomView : public QGraphicsView {
public:
    ZoomView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    }
protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) {
            const double scaleFactor = 1.15;
            if (event->angleDelta().y() > 0) scale(scaleFactor, scaleFactor);
            else scale(1.0/scaleFactor, 1.0/scaleFactor);
            event->accept();
        } else {
            QGraphicsView::wheelEvent(event);
        }
    }
};

HuffmanWidget::HuffmanWidget(QWidget *parent)
    : QMainWindow(parent)
{
    // sample data
    m_items = {{"A",45},{"B",13},{"C",12},{"D",16},{"E",9},{"F",5}};

    setupUi();
    prepareInitialNodes();

    HuffmanBuilder builder(m_items);
    m_steps = builder.buildSteps();

    m_timer = new QTimer(this);
    m_timer->setInterval(800);
    connect(m_timer, &QTimer::timeout, this, &HuffmanWidget::autoplayStep);
}

HuffmanWidget::~HuffmanWidget() {}

void HuffmanWidget::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    m_scene = new QGraphicsScene(this);
    // use ZoomView to get wheel zoom
    m_view = new ZoomView(this);
    m_view->setScene(m_scene);

    m_stepBtn = new QPushButton(tr("Step"), this);
    m_playBtn = new QPushButton(tr("Play"), this);
    m_resetBtn = new QPushButton(tr("Reset"), this);

    connect(m_stepBtn, &QPushButton::clicked, this, &HuffmanWidget::onStep);
    connect(m_playBtn, &QPushButton::clicked, this, &HuffmanWidget::onPlayPause);
    connect(m_resetBtn, &QPushButton::clicked, this, &HuffmanWidget::onReset);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(m_stepBtn);
    hbox->addWidget(m_playBtn);
    hbox->addWidget(m_resetBtn);
    hbox->addStretch();
    QLabel *zl = new QLabel(tr("Zoom:"), this);
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(20, 400);
    m_zoomSlider->setValue(100);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &HuffmanWidget::onZoomChanged);
    hbox->addWidget(zl);
    hbox->addWidget(m_zoomSlider);

    QVBoxLayout *vbox = new QVBoxLayout(central);
    vbox->addWidget(m_view, 1);
    vbox->addLayout(hbox);
}

void HuffmanWidget::prepareInitialNodes()
{
    m_scene->clear();
    m_itemMap.clear();
    m_currentStep = 0;

    // create leaf visual items (ids 0..n-1)
    for (int i = 0; i < m_items.size(); ++i) {
        QString label = m_items[i].first;
        int w = m_items[i].second;
        QGraphicsItemGroup *group = new QGraphicsItemGroup();
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(-30, -30, 60, 60);
        ellipse->setBrush(QBrush(QColor(100,150,240)));
        ellipse->setPen(QPen(Qt::black));
        QGraphicsTextItem *t1 = new QGraphicsTextItem(label);
        t1->setDefaultTextColor(Qt::white);
        t1->setPos(-10, -10);
        QGraphicsTextItem *t2 = new QGraphicsTextItem(QString::number(w));
        t2->setDefaultTextColor(Qt::black);
        t2->setPos(-10, 10);
        group->addToGroup(ellipse);
        group->addToGroup(t1);
        group->addToGroup(t2);
        m_scene->addItem(group);

        group->setData(0, i);
        // tooltip with label and weight
        group->setToolTip(QString("%1 : %2").arg(label).arg(w));
        m_itemMap.insert(i, group);
    }

    // clear edges list
    clearEdges();

    // perform initial layout (no steps applied)
    layoutAndAnimate(0);
}

void HuffmanWidget::onStep()
{
    if (m_currentStep >= m_steps.size()) return;
    performStep(m_steps[m_currentStep]);
    m_currentStep++;
}

void HuffmanWidget::performStep(const HuffStep &step)
{
    // Find graphics items for a and b by their id
    int idA = step.a->id;
    int idB = step.b->id;

    QGraphicsItem *itemA = m_itemMap.value(idA, nullptr);
    QGraphicsItem *itemB = m_itemMap.value(idB, nullptr);

    // create parent visual (parent id provided by builder)
    int parentId = step.parent->id;

    // highlight A and B
    if (itemA) {
        auto groupshape = static_cast<QGraphicsItemGroup*>(itemA);
        // change ellipse color
        foreach(QGraphicsItem *child, groupshape->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(240,120,100))); // highlight
            }
        }
    }
    if (itemB) {
        auto groupshape = static_cast<QGraphicsItemGroup*>(itemB);
        foreach(QGraphicsItem *child, groupshape->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(240,120,100))); // highlight
            }
        }
    }

    // create parent group immediately (content only), position will be set by layout
    QGraphicsItemGroup *parentGroup = new QGraphicsItemGroup();
    QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(-30, -30, 60, 60);
    // gradient for parent
    QLinearGradient lg(-30, -30, 30, 30);
    lg.setColorAt(0, QColor(140,220,180));
    lg.setColorAt(1, QColor(80,180,120));
    ellipse->setBrush(lg);
    ellipse->setPen(QPen(Qt::black));
    QGraphicsTextItem *t = new QGraphicsTextItem(QString::number(step.parent->weight));
    t->setPos(-10, -10);
    parentGroup->addToGroup(ellipse);
    parentGroup->addToGroup(t);
    parentGroup->setToolTip(QString("weight: %1").arg(step.parent->weight));
    m_scene->addItem(parentGroup);
    m_itemMap.insert(parentId, parentGroup);

    // dim A and B after creating parent visual
    if (itemA) {
        auto groupshape = static_cast<QGraphicsItemGroup*>(itemA);
        foreach(QGraphicsItem *child, groupshape->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(180,180,180)));
            }
        }
    }
    if (itemB) {
        auto groupshape = static_cast<QGraphicsItemGroup*>(itemB);
        foreach(QGraphicsItem *child, groupshape->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(180,180,180)));
            }
        }
    }

    // After creating the new parent node, recompute layout for stepsDone = m_currentStep+1
    layoutAndAnimate(m_currentStep + 1);
}

// remove existing edge items
void HuffmanWidget::clearEdges()
{
    for (auto *li : m_edgeItems) {
        if (li && li->scene()) li->scene()->removeItem(li);
        delete li;
    }
    m_edgeItems.clear();
}

// draw edges for current steps
void HuffmanWidget::drawEdges(int stepsDone)
{
    clearEdges();
    for (int i = 0; i < stepsDone && i < m_steps.size(); ++i) {
        const HuffStep &s = m_steps[i];
        int pid = s.parent->id;
        int aid = s.a->id;
        int bid = s.b->id;
        QGraphicsItem *pItem = m_itemMap.value(pid, nullptr);
        QGraphicsItem *aItem = m_itemMap.value(aid, nullptr);
        QGraphicsItem *bItem = m_itemMap.value(bid, nullptr);
        if (pItem && aItem) {
            QPointF start = pItem->pos();
            QPointF end = aItem->pos();
            QPointF c1 = start + QPointF(0, -40);
            QPointF c2 = end + QPointF(0, 40);
            QPainterPath path(start);
            path.cubicTo(c1, c2, end);
            QGraphicsPathItem *pi = new QGraphicsPathItem(path);
            QPen pen(Qt::black);
            pen.setWidth(2);
            pi->setPen(pen);
            pi->setZValue(-1);
            m_scene->addItem(pi);
            m_edgeItems.append(pi);
            // arrow at end
            QPointF dir = (end - c2);
            double len = std::hypot(dir.x(), dir.y());
            if (len > 0.1) {
                dir /= len;
                QPointF p1 = end - dir * 12 + QPointF(-dir.y()*6, dir.x()*6);
                QPointF p2 = end - dir * 12 + QPointF(dir.y()*6, -dir.x()*6);
                QPolygonF tri; tri << end << p1 << p2;
                QGraphicsPolygonItem *arrow = new QGraphicsPolygonItem(tri);
                arrow->setBrush(Qt::black);
                arrow->setZValue(-1);
                m_scene->addItem(arrow);
                m_edgeItems.append(arrow);
            }
        }
        if (pItem && bItem) {
            QPointF start = pItem->pos();
            QPointF end = bItem->pos();
            QPointF c1 = start + QPointF(0, -40);
            QPointF c2 = end + QPointF(0, 40);
            QPainterPath path(start);
            path.cubicTo(c1, c2, end);
            QGraphicsPathItem *pi = new QGraphicsPathItem(path);
            QPen pen(Qt::black);
            pen.setWidth(2);
            pi->setPen(pen);
            pi->setZValue(-1);
            m_scene->addItem(pi);
            m_edgeItems.append(pi);
            QPointF dir = (end - c2);
            double len = std::hypot(dir.x(), dir.y());
            if (len > 0.1) {
                dir /= len;
                QPointF p1 = end - dir * 12 + QPointF(-dir.y()*6, dir.x()*6);
                QPointF p2 = end - dir * 12 + QPointF(dir.y()*6, -dir.x()*6);
                QPolygonF tri; tri << end << p1 << p2;
                QGraphicsPolygonItem *arrow = new QGraphicsPolygonItem(tri);
                arrow->setBrush(Qt::black);
                arrow->setZValue(-1);
                m_scene->addItem(arrow);
                m_edgeItems.append(arrow);
            }
        }
    }
}

// Compute layout for forest after 'stepsDone' steps and animate items to new positions
void HuffmanWidget::layoutAndAnimate(int stepsDone)
{
    // Build lightweight layout nodes for current forest
    struct LNode { int id; int weight; LNode *left{nullptr}, *right{nullptr}; double x{0}, y{0}, width{0}; };
    QVector<LNode*> forest;
    QMap<int, LNode*> nodeMap;
    // create initial leaves
    for (int i = 0; i < m_items.size(); ++i) {
        LNode *ln = new LNode(); ln->id = i; ln->weight = m_items[i].second; ln->width = 60; nodeMap.insert(i, ln); forest.append(ln);
    }

    // apply merges up to stepsDone
    for (int i = 0; i < stepsDone && i < m_steps.size(); ++i) {
        const HuffStep &s = m_steps[i];
        int aId = s.a->id; int bId = s.b->id; int pId = s.parent->id;
        LNode *a = nodeMap.value(aId, nullptr);
        LNode *b = nodeMap.value(bId, nullptr);
        LNode *p = new LNode(); p->id = pId; p->weight = s.parent->weight;
        p->left = a; p->right = b;
        // remove a and b from forest
        for (int j = 0; j < forest.size(); ++j) {
            if (forest[j] == a || forest[j] == b) { forest.removeAt(j); --j; }
        }
        forest.append(p);
        nodeMap.insert(pId, p);
    }

    // compute subtree widths recursively
    std::function<double(LNode*)> computeWidth = [&](LNode* n)->double {
        if (!n) return 0;
        if (!n->left && !n->right) { n->width = 60; return n->width; }
        double leftW = computeWidth(n->left);
        double rightW = computeWidth(n->right);
        double spacing = 20;
        n->width = leftW + rightW + spacing;
        return n->width;
    };

    // depth map for y positions
    std::function<int(LNode*)> computeDepth = [&](LNode* n)->int {
        if (!n) return 0;
        if (!n->left && !n->right) return 0;
        return 1 + qMax(computeDepth(n->left), computeDepth(n->right));
    };

    // assign x positions starting from left boundary
    std::function<void(LNode*, double, int, double)> assignPos = [&](LNode* n, double left, int depth, double levelHeight){
        if (!n) return;
        if (!n->left && !n->right) {
            n->x = left + n->width/2;
            n->y = 500 - depth * levelHeight;
            return;
        }
        double spacing = 20;
        double leftStart = left;
        assignPos(n->left, leftStart, depth+1, levelHeight);
        assignPos(n->right, leftStart + (n->left ? n->left->width : 0) + spacing, depth+1, levelHeight);
        // parent x is center of children
        double lx = n->left ? n->left->x : 0;
        double rx = n->right ? n->right->x : lx;
        n->x = (lx + rx) / 2.0;
        n->y = 500 - depth * levelHeight;
    };

    // compute widths for each root
    for (LNode* r : forest) computeWidth(r);

    // total width
    double totalW = 0;
    for (LNode* r : forest) totalW += r->width + 20;
    if (totalW < 400) totalW = 400;
    double left = 50;
    double levelH = 100.0;
    // assign positions for each root sequentially
    for (LNode* r : forest) {
        assignPos(r, left, 0, levelH);
        left += r->width + 20;
    }

    // animate existing graphics items to new positions and scale based on weight
    QVector<QAbstractAnimation*> anims;
    // find global max weight for scaling
    int maxW = 1;
    for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it) if (it.value()) maxW = qMax(maxW, it.value()->weight);
    for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it) {
        int id = it.key();
        LNode* ln = it.value();
        QGraphicsItem *g = m_itemMap.value(id, nullptr);
        if (!g) continue;
        QPointF cur = g->pos();
        QPointF target(ln->x, ln->y);
        if ((cur - target).manhattanLength() < 1.0) { g->setPos(target); continue; }
        QVariantAnimation *va = new QVariantAnimation(this);
        va->setStartValue(cur);
        va->setEndValue(target);
        va->setDuration(500);
        connect(va, &QVariantAnimation::valueChanged, this, [g](const QVariant &v){ g->setPos(v.toPointF()); });
        anims.append(va);
        // scale animation
        double scaleTarget = 0.8 + 1.2 * (double(ln->weight) / double(maxW));
        QVariantAnimation *sa = new QVariantAnimation(this);
        sa->setStartValue(g->scale());
        sa->setEndValue(scaleTarget);
        sa->setDuration(500);
        connect(sa, &QVariantAnimation::valueChanged, this, [g](const QVariant &v){ g->setScale(v.toDouble()); });
        anims.append(sa);
    }

    // play animations in parallel
    QParallelAnimationGroup *pg = new QParallelAnimationGroup(this);
    for (auto a : anims) pg->addAnimation(a);
    connect(pg, &QParallelAnimationGroup::finished, this, [this, pg, stepsDone, nodeMap]() mutable {
        // after animation finishes, draw edges according to stepsDone
        drawEdges(stepsDone);
        // remove from active animations list
        m_activeAnims.removeAll(pg);
        pg->deleteLater();
        // clean up layout nodes
        for (auto v : nodeMap) delete v;
    });
    if (pg->animationCount() > 0) {
        // track active animation so we can stop it on reset
        m_activeAnims.append(pg);
        pg->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        // no animations; just draw edges
        drawEdges(stepsDone);
        for (auto v : nodeMap) delete v;
    }
    
    // update scene rect and center
    QRectF bounds = m_scene->itemsBoundingRect();
    m_scene->setSceneRect(bounds.adjusted(-50,-50,50,50));
    m_view->fitInView(bounds, Qt::KeepAspectRatio);
}

void HuffmanWidget::onPlayPause()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        m_playBtn->setText(tr("Play"));
    } else {
        m_timer->start();
        m_playBtn->setText(tr("Pause"));
    }
}

void HuffmanWidget::autoplayStep()
{
    if (m_currentStep >= m_steps.size()) {
        m_timer->stop();
        m_playBtn->setText(tr("Play"));
        return;
    }
    onStep();
}

void HuffmanWidget::onReset()
{
    m_timer->stop();
    m_playBtn->setText(tr("Play"));
    // stop and clear active animations to avoid use-after-free when clearing the scene
    for (QAbstractAnimation *a : m_activeAnims) {
        if (a->state() == QAbstractAnimation::Running) a->stop();
        a->deleteLater();
    }
    m_activeAnims.clear();

    // clear edges and items safely
    clearEdges();
    prepareInitialNodes();
    HuffmanBuilder builder(m_items);
    m_steps = builder.buildSteps();
}

void HuffmanWidget::onZoomChanged(int value)
{
    if (!m_view) return;
    double f = double(value) / 100.0;
    m_view->resetTransform();
    m_view->scale(f, f);
}
