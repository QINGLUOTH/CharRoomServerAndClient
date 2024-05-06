#include "mainwindow.h"

#include <QApplication>

// 聊天管理系统
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int i = a.exec();
    return i;
}
