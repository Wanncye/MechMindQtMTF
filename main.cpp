#include "mainwindow.h"

#include <QApplication>
#include "mdump.h"
MiniDumper dump;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
