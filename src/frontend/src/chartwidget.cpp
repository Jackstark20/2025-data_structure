#include "chartwidget.h"
#include "ui_chartwidget.h"
#include <QDebug>
#include "src/backend/EncodingUtils.h" // 用于字符编码转换

ChartWidget::ChartWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartWidget),
    m_chart(new QChart),
    m_series(new QBarSeries),
    m_axisX(new QBarCategoryAxis),
    m_axisY(new QValueAxis) {
    ui->setupUi(this);

    // 初始化图表
    m_chart->addSeries(m_series);
    m_chart->setTitle("词频/字节频率统计");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    // 显示图表
    QChartView *chartView = new QChartView(m_chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout->addWidget(chartView);
}

ChartWidget::~ChartWidget() {
    delete ui;
}

void ChartWidget::clearChart() {
    m_series->clear();
    m_axisX->clear();
}

void ChartWidget::updateTextFrequency(const std::unordered_map<char32_t, size_t>& freqMap) {
    clearChart();
    QBarSet *set = new QBarSet("字符频率");
    QStringList categories;

    // 转换字符为可显示格式（处理UTF-8）
    for (const auto& pair : freqMap) {
        wchar_t wch = static_cast<wchar_t>(pair.first);
        QString charStr = QString::fromUtf8(wstring_to_utf8(std::wstring(1, wch)).c_str());
        categories << (charStr.isEmpty() ? " " : charStr); // 处理空字符
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

    // 直接显示字节值（0-255）
    for (const auto& pair : freqVec) {
        categories << QString("0x%1").arg(pair.first, 2, 16, QChar('0')).toUpper();
        *set << static_cast<double>(pair.second);
    }

    m_series->append(set);
    m_axisX->append(categories);
    m_axisY->setTitleText("出现次数");
    m_chart->setTitle("图片字节频率分布");
}