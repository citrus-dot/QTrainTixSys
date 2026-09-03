#ifndef QUERYPAGE_H
#define QUERYPAGE_H

#include <QWidget>
#include "trainsystem.h"

class QListWidgetItem;
class SeatMapPopup;

QT_BEGIN_NAMESPACE
namespace Ui { class QueryPage; }
QT_END_NAMESPACE

// 查询页：选择班次查看余票、可选座位与停靠站
class QueryPage : public QWidget
{
    Q_OBJECT
public:
    explicit QueryPage(QWidget *parent = nullptr);
    ~QueryPage();

    void setSystem(const TrainSystem *system);
    void refresh(); // 重新加载班次列表并刷新显示

private slots:
    void onTrainChanged();
    void onSeatClicked(QListWidgetItem *item);
    void onStopClicked(QListWidgetItem *item);

private:
    Ui::QueryPage *ui;
    const TrainSystem *m_system = nullptr;
    SeatMapPopup *m_seatPopup = nullptr;
};

#endif // QUERYPAGE_H