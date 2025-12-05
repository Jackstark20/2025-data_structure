#include "chartwidget.h"

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <cstdlib>
#include <ctime>

ChartWidget::ChartWidget(QWidget *parent)
    : QMainWindow(parent)
{
    // sample words
    m_words = {"apple", "banana", "cherry", "date", "elder", "fig", "grape"};

    // seed random
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // initial random data
    QVector<qreal> values;
    for (int i = 0; i < m_words.size(); ++i) {
        int v = 20 + (std::rand() % 80); // 20..99
        values.append(v);
    }

    m_barSet = new QBarSet("Frequency");
    for (qreal v : values) {
        m_barSet->append(v);
    }

    m_series = new QBarSeries();
    m_series->append(m_barSet);

    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_chart->setTitle("实时词频 (模拟数据)");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    // X axis (categories)
    m_axisX = new QBarCategoryAxis();
    m_axisX->append(m_words);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    // Y axis
    m_axisY = new QValueAxis();
    m_axisY->setRange(0, 140);
    m_axisY->setLabelFormat("%d");
    m_axisY->setTitleText("次数");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    setCentralWidget(m_chartView);

    // timer for updates
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ChartWidget::updateData);
    m_timer->start(600); // 600 ms update
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::updateData()
{
    // Slightly change each word's frequency randomly
    qreal maxVal = 0;
    for (int i = 0; i < m_words.size(); ++i) {
        // current value
        qreal cur = m_barSet->at(i);
        int delta = (std::rand() % 21) - 10; // -10..+10
        qreal next = cur + delta;
        if (next < 0) next = 0;
        m_barSet->replace(i, next);
        if (next > maxVal) maxVal = next;
    }

    // adjust Y-axis top a bit above max
    int top = static_cast<int>(std::ceil(maxVal * 1.2));
    if (top < 10) top = 10;
    m_axisY->setRange(0, top);

    // chart will repaint automatically; keep animation
}
