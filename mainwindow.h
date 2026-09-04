#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "trainsystem.h"

class QLabel;

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
    void onTrainDoubleClicked();
    void onSeatTable();
    void onQuery();
    void onAbout();
    void onPageSelected(int index);
    void onTrainSelectionChanged(const QString &no);

private:
    void refreshTrainList();
    void updateStats();
    Train *currentTrain();

    Ui::MainWindow *ui;
    TrainSystem m_system;
    QString m_filePath;
    QLabel *m_statsLabel = nullptr;
};

#endif // MAINWINDOW_H
