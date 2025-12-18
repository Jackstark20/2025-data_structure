#ifndef CHARACTERFREQUENCYCHART_H
#define CHARACTERFREQUENCYCHART_H

#include <QWidget>
#include <QMap>
#include <QChar>
#include <QTimer>
#include <QList>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class CharacterFrequencyChart : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterFrequencyChart(QWidget *parent = nullptr);
    ~CharacterFrequencyChart();

    void reset();
    void updateFrequency(const QMap<QChar, int> &frequencyMap);
    void setShowAllCharacters(bool showAll);
    void setRefreshInterval(int ms);

private slots:
    void updateChart();

private:
    void initChart();
    void updateAxes();

    QChartView *m_chartView;
    QChart *m_chart;
    QBarSeries *m_series;
    QList<QBarSet*> m_barSets;
    QBarCategoryAxis *m_categoryAxis;
    QValueAxis *m_valueAxis;
    QTimer *m_updateTimer;
    QMap<QChar, int> m_currentFrequencies;
    bool m_showAllCharacters;
    int m_refreshInterval;
};

#endif // CHARACTERFREQUENCYCHART_H