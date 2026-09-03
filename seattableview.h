#ifndef SEATTABLEVIEW_H
#define SEATTABLEVIEW_H

#include <QWidget>
#include "train.h"

class QTableWidget;

// 座位登记表组件：显示当前选中班次的已售座位
class SeatTableView : public QWidget
{
    Q_OBJECT
public:
    explicit SeatTableView(QWidget *parent = nullptr);

    void setTrain(const Train *train); // nullptr 清空

private:
    QTableWidget *m_table;
};

#endif // SEATTABLEVIEW_H