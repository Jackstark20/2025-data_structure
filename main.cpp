#include <QApplication>
#include "chartwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChartWidget w;
    w.resize(900, 600);
    w.show();
    return a.exec();
}
