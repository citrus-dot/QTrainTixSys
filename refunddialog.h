#ifndef REFUNDDIALOG_H
#define REFUNDDIALOG_H

#include "fadedialog.h"
#include "train.h"

class SeatStatusTable;

// 退票弹窗：座位状态表格，选中已售座位后确认退票
class RefundDialog : public FadeDialog
{
    Q_OBJECT
public:
    explicit RefundDialog(QWidget *parent = nullptr);

    void setTrain(Train *train);

signals:
    void dataChanged(); // 退票后数据变化

private:
    void onRefund();

    SeatStatusTable *m_table;
    Train *m_train = nullptr;
};

#endif // REFUNDDIALOG_H
