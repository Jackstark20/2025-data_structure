#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "chartwidget.h"  // 前端图表控件
#include "HuffmanTree.h"  // 后端头文件

// 不需要UI类，纯代码构建主窗口
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_selectFileBtn_clicked();
    void on_processBtn_clicked();

private:
    // 直接定义窗口控件成员
    QWidget *m_centralWidget;    // 中心容器
    QVBoxLayout *m_mainLayout;   // 主布局
    QHBoxLayout *m_btnLayout;    // 按钮布局
    QLineEdit *m_filePathEdit;   // 文件路径输入框
    QPushButton *m_selectFileBtn;// 选择文件按钮
    QPushButton *m_processBtn;   // 处理文件按钮
    ChartWidget *m_chartWidget;  // 图表控件

    std::string m_selectedFilePath;  // 选中的文件路径
};

#endif // MAINWINDOW_H