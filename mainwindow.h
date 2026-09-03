#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "trainsystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onAddTrain();
    void onRemoveTrain();
    void onSellTicket();
    void onRefundTicket();
    void onQuery();
    void onAbout();

private:
    void refreshTrainList();
    void refreshSeatTable();
    Train *currentTrain();

    Ui::MainWindow *ui;
    TrainSystem m_system;
    QString m_filePath;
};

#endif // MAINWINDOW_H
