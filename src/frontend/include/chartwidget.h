#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <vector>
#include <unordered_map>
#include "src/backend/HuffmanTree.h" // 引用后端头文件

QT_BEGIN_NAMESPACE
namespace Ui { class ChartWidget; }
QT_END_NAMESPACE

class ChartWidget : public QWidget {
    Q_OBJECT

public:
    ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    // 更新词频图表（文本模式）
    void updateTextFrequency(const std::unordered_map<char32_t, size_t>& freqMap);
    // 更新字节频率图表（图片模式）
    void updateByteFrequency(const std::vector<std::pair<BYTE, int>>& freqVec);

private:
    Ui::ChartWidget *ui;
    QChart *m_chart;
    QBarSeries *m_series;
    QBarCategoryAxis *m_axisX;
    QValueAxis *m_axisY;

    // 清除现有图表数据
    void clearChart();
};
#endif // CHARTWIDGET_H