#include "chartwidget.h"
#include <QDebug>
#include "EncodingUtils.h"  // 后端编码工具

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
    // 1. 初始化布局（替代UI文件的布局）
    m_mainLayout = new QVBoxLayout(this);
    this->setLayout(m_mainLayout);

    // 2. 初始化图表控件（替代UI文件的图表视图）
    m_chart = new QChart();
    m_series = new QBarSeries();
    m_axisX = new QBarCategoryAxis();
    m_axisY = new QValueAxis();

    // 3. 配置图表
    m_chart->addSeries(m_series);
    m_chart->setTitle("词频/字节频率统计");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    // 4. 图表视图添加到布局
    QChartView *chartView = new QChartView(m_chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    m_mainLayout->addWidget(chartView);
}

ChartWidget::~ChartWidget() {
    // 手动释放控件（避免内存泄漏）
    delete m_series;
    delete m_axisX;
    delete m_axisY;
    delete m_chart;
    // 布局由父控件自动释放，无需手动删除
}

void ChartWidget::clearChart() {
    m_series->clear();
    m_axisX->clear();
}

void ChartWidget::updateTextFrequency(const std::unordered_map<char32_t, size_t>& freqMap) {
    clearChart();
    QBarSet *set = new QBarSet("字符频率");
    QStringList categories;

    // 转换UTF-8字符为QString（兼容多字节字符）
    for (const auto& pair : freqMap) {
        wchar_t wch = static_cast<wchar_t>(pair.first);
        std::string utf8Str = wstring_to_utf8(std::wstring(1, wch));
        QString charStr = QString::fromUtf8(utf8Str.c_str());
        // 处理空字符显示
        categories << (charStr.isEmpty() ? QString("(空字符)") : charStr);
        *set << static_cast<double>(pair.second);
    }

    m_series->append(set);
    m_axisX->append(categories);
    m_axisY->setTitleText("出现次数");
    m_chart->setTitle("文本字符频率分布");
}

void ChartWidget::updateByteFrequency(const std::vector<std::pair<BYTE, int>>& freqVec) {
    clearChart();
    QBarSet *set = new QBarSet("字节频率");
    QStringList categories;

    // 显示字节的十六进制格式（图片字节常用格式）
    for (const auto& pair : freqVec) {
        categories << QString("0x%1").arg(pair.first, 2, 16, QChar('0')).toUpper();
        *set << static_cast<double>(pair.second);
    }

    m_series->append(set);
    m_axisX->append(categories);
    m_axisY->setTitleText("出现次数");
    m_chart->setTitle("图片字节频率分布");
}