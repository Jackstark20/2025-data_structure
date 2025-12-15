#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <vector>
#include <unordered_map>
#include "HuffmanTree.h"  // 后端头文件（已配置路径）

// 不需要UI类，直接纯代码构建界面
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget() override;

    // 更新词频图表（文本模式）
    void updateTextFrequency(const std::unordered_map<char32_t, size_t>& freqMap);
    // 更新字节频率图表（图片模式）
    void updateByteFrequency(const std::vector<std::pair<BYTE, int>>& freqVec);

private:
    // 移除UI指针，直接定义控件成员
    QChart *m_chart;
    QBarSeries *m_series;
    QBarCategoryAxis *m_axisX;
    QValueAxis *m_axisY;
    QVBoxLayout *m_mainLayout;  // 布局控件

    // 清除现有图表数据
    void clearChart();
};

#endif // CHARTWIDGET_H