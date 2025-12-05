#include <QApplication>
#include "huffmanwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HuffmanWidget w;
    w.resize(1000, 700);
    w.show();
    return a.exec();
}
