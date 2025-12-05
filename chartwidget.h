#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QTimer>

// Note: avoid forcing a namespace import here to keep compatibility across Qt versions

class ChartWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

private slots:
    void updateData();

private:
    QStringList m_words;
    QBarSet *m_barSet{nullptr};
    QBarSeries *m_series{nullptr};
    QChart *m_chart{nullptr};
    QChartView *m_chartView{nullptr};
    QValueAxis *m_axisY{nullptr};
    QBarCategoryAxis *m_axisX{nullptr};
    QTimer *m_timer{nullptr};
};

#endif // CHARTWIDGET_H
