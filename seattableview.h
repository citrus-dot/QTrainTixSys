#ifndef SEATTABLEVIEW_H
#define SEATTABLEVIEW_H

#include <QWidget>
#include "train.h"

class QLabel;
class QTableWidget;

// 座位登记表组件：标题 + 当前班次已售座位列表 + 空状态引导
class SeatTableView : public QWidget
{
    Q_OBJECT
public:
    explicit SeatTableView(QWidget *parent = nullptr);

    void setTrain(const Train *train); // nullptr 清空

private:
    QLabel *m_title;
    QLabel *m_emptyHint;
    QTableWidget *m_table;
};

#endif // SEATTABLEVIEW_H
