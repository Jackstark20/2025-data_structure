#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chartwidget.h"
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectFileBtn_clicked();
    void on_processBtn_clicked();

private:
    Ui::MainWindow *ui;
    ChartWidget *m_chartWidget;
    std::string m_selectedFilePath;
};
#endif // MAINWINDOW_H