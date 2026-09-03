#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 强制浅色外观，避免 macOS 深色模式下未覆盖控件显示深色
    QPalette pal = a.palette();
    pal.setColor(QPalette::Window, QColor("#F4F6FA"));
    pal.setColor(QPalette::WindowText, QColor("#1B2430"));
    pal.setColor(QPalette::Base, QColor("#FFFFFF"));
    pal.setColor(QPalette::AlternateBase, QColor("#F8FAFD"));
    pal.setColor(QPalette::Text, QColor("#1B2430"));
    pal.setColor(QPalette::Button, QColor("#FFFFFF"));
    pal.setColor(QPalette::ButtonText, QColor("#1B2430"));
    pal.setColor(QPalette::Highlight, QColor("#2F6FED"));
    pal.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    a.setPalette(pal);

    QFile qss(":/theme.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));

    MainWindow w;
    w.show();
    return QApplication::exec();
}
