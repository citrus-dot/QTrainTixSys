#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "trainsystem.h"

class QLabel;
class QAbstractAnimation;
class QCloseEvent;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 直接加载数据文件（不弹文件对话框），供自动化冒烟测试使用
    bool loadFile(const QString &path);

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onAddTrain();
    void onEditTrain();
    void onRemoveTrain();
    void onSellTicket();
    void onRefundTicket();
    void onTrainDoubleClicked();
    void onSeatTable();
    void onAbout();
    void onPageSelected(int index);
    void onTrainSelectionChanged(const QString &no);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void refreshTrainList();
    void updateStats();
    void animatePageIn(QWidget *page); // 切页过渡：淡入 + 轻微上浮
    void markDirty();                  // 数据改动时置脏
    void restoreWindowState();         // 恢复窗口几何并自动打开上次文件
    Train *currentTrain();

    Ui::MainWindow *ui;
    TrainSystem m_system;
    QString m_filePath;
    bool m_dirty = false;              // 有未保存改动（关闭时提示）
    QLabel *m_statsLabel = nullptr;
    QPointer<QAbstractAnimation> m_pageAnim; // 进行中的切页动画（快速连点时抢占）
};

#endif // MAINWINDOW_H