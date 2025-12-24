#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    // 初始化Qt应用程序
    QApplication a(argc, argv);
    // 创建主窗口对象
    MainWindow w;
    // 显示主窗口
    w.show();
    //循环直到用户关闭窗口
    return a.exec();
}