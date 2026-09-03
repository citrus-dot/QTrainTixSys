#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/theme.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
        a.setStyleSheet(qss.readAll());

    MainWindow w;
    w.show();
    return QApplication::exec();
}
