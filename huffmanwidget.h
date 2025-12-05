#ifndef HUFFMANWIDGET_H
#define HUFFMANWIDGET_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>
#include <QTimer>
#include <QSlider>
#include <QAbstractAnimation>
#include "huffmantree.h"

class HuffmanWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit HuffmanWidget(QWidget *parent = nullptr);
    ~HuffmanWidget();

private slots:
    void onStep();
    void onPlayPause();
    void onReset();
    void autoplayStep();

private:
    void setupUi();
    // layout and drawing helpers
    void layoutAndAnimate(int stepsDone);
    void clearEdges();
    void drawEdges(int stepsDone);
    void onZoomChanged(int value);

    void prepareInitialNodes();
    void performStep(const HuffStep &step);

    QGraphicsView *m_view{nullptr};
    QGraphicsScene *m_scene{nullptr};
    QPushButton *m_stepBtn{nullptr};
    QPushButton *m_playBtn{nullptr};
    QPushButton *m_resetBtn{nullptr};
    QTimer *m_timer{nullptr};
    QSlider *m_zoomSlider{nullptr};

    QVector<QPair<QString,int>> m_items;
    QVector<HuffStep> m_steps;
    int m_currentStep{0};
    // mapping node id -> QGraphicsItem group
    QMap<int, QGraphicsItem*> m_itemMap;
    QVector<QGraphicsItem*> m_edgeItems;
    QVector<QAbstractAnimation*> m_activeAnims;
};

#endif // HUFFMANWIDGET_H
