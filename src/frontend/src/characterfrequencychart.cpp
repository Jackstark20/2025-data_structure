#include "characterfrequencychart.h"
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

CharacterFrequencyChart::CharacterFrequencyChart(QWidget *parent)
    : QWidget(parent),
      m_chartView(nullptr),
      m_chart(nullptr),
      m_series(nullptr),
      m_categoryAxis(nullptr),
      m_valueAxis(nullptr),
      m_currentFrequencies(),
      m_showAllCharacters(false),
      m_refreshInterval(100)
{
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &CharacterFrequencyChart::updateChart);
    
    initChart();
    m_updateTimer->start(m_refreshInterval);
}

CharacterFrequencyChart::~CharacterFrequencyChart()
{
    // 停止定时器
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    // 清理资源
    // m_chartView 有 parent (this)，会被自动删除，不需要手动 delete
    // 但需要先清理 chart 相关的资源
    qDeleteAll(m_barSets);
    m_barSets.clear();
    
    // 删除轴（它们不属于 chart 的子对象）
    delete m_categoryAxis;
    delete m_valueAxis;
    
    // 删除 series（它不属于 chart 的子对象）
    delete m_series;
    
    // chart 和 chartView 会被 Qt 的父子关系自动删除
    // 但为了安全，我们显式清理
    if (m_chart) {
        m_chart->removeAllSeries(); // 移除所有 series
    }
}

void CharacterFrequencyChart::reset()
{
    m_currentFrequencies.clear();
    updateChart();
}

void CharacterFrequencyChart::updateFrequency(const QMap<QChar, int> &frequencyMap)
{
    // 安全检查：确保图表已初始化
    if (!m_chart || !m_series) {
        return;
    }
    m_currentFrequencies = frequencyMap;
}

void CharacterFrequencyChart::setShowAllCharacters(bool showAll)
{
    m_showAllCharacters = showAll;
    updateChart();
}

void CharacterFrequencyChart::setRefreshInterval(int ms)
{
    m_refreshInterval = ms;
    if (m_updateTimer) {
        m_updateTimer->setInterval(ms);
    }
}

void CharacterFrequencyChart::updateChart()
{
    // 安全检查：确保所有必要的组件都已初始化
    if (!m_series || !m_categoryAxis || !m_valueAxis || !m_chart)
        return;
        
    // 清除现有数据
    m_series->clear();
    qDeleteAll(m_barSets);
    m_barSets.clear();
    
    // 添加新的数据
    QBarSet *barSet = new QBarSet("频率");
    QStringList categories;
    
    for (auto it = m_currentFrequencies.constBegin(); it != m_currentFrequencies.constEnd(); ++it) {
        if (it.value() > 0 || m_showAllCharacters) {
            categories << QString(it.key());
            barSet->append(it.value());
        }
    }
    
    // 只有当有数据时才添加到 series
    if (barSet->count() > 0) {
        m_series->append(barSet);
        m_barSets << barSet;
        updateAxes();
    } else {
        // 如果没有数据，删除空的 barSet
        delete barSet;
    }
}

void CharacterFrequencyChart::initChart()
{
    // 创建图表组件
    m_chart = new QChart();
    m_chartView = new QChartView(m_chart, this);
    m_series = new QBarSeries(this);
    m_categoryAxis = new QBarCategoryAxis();
    m_valueAxis = new QValueAxis();
    
    // 设置图表属性
    m_chart->setTitle("字符频率统计");
    m_chart->addSeries(m_series);
    
    // 设置轴
    m_chart->addAxis(m_categoryAxis, Qt::AlignBottom);
    m_chart->addAxis(m_valueAxis, Qt::AlignLeft);
    m_series->attachAxis(m_categoryAxis);
    m_series->attachAxis(m_valueAxis);
    
    // 设置布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    setLayout(layout);
}

void CharacterFrequencyChart::updateAxes()
{
    if (!m_categoryAxis || !m_valueAxis)
        return;
        
    // 更新分类轴
    QStringList categories;
    for (auto it = m_currentFrequencies.constBegin(); it != m_currentFrequencies.constEnd(); ++it) {
        if (it.value() > 0 || m_showAllCharacters) {
            categories << QString(it.key());
        }
    }
    m_categoryAxis->clear();
    m_categoryAxis->append(categories);
    
    // 更新值轴
    int maxValue = 0;
    for (auto it = m_currentFrequencies.constBegin(); it != m_currentFrequencies.constEnd(); ++it) {
        if (it.value() > maxValue) {
            maxValue = it.value();
        }
    }
    m_valueAxis->setRange(0, maxValue + 1);
}