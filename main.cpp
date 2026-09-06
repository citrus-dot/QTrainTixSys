#include "mainwindow.h"
#include "palette.h"

#include <QApplication>
#include <QFile>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("列车售票系统");

    // 强制浅色外观，避免 macOS 深色模式下未覆盖控件显示深色
    QPalette pal = a.palette();
    pal.setColor(QPalette::Window, Palette::kWindowBg);
    pal.setColor(QPalette::WindowText, Palette::kText);
    pal.setColor(QPalette::Base, Palette::kWhite);
    pal.setColor(QPalette::AlternateBase, Palette::kTableAlt);
    pal.setColor(QPalette::Text, Palette::kText);
    pal.setColor(QPalette::Button, Palette::kWhite);
    pal.setColor(QPalette::ButtonText, Palette::kText);
    pal.setColor(QPalette::Highlight, Palette::kBrand);
    pal.setColor(QPalette::HighlightedText, Palette::kWhite);
    a.setPalette(pal);

    QFile qss(":/theme.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));

    MainWindow w;
    w.show();
    return QApplication::exec();
}
