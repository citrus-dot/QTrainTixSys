#ifndef SELLDIALOG_H
#define SELLDIALOG_H

#include "fadedialog.h"
#include "train.h"

class SeatStatusTable;

// 售票弹窗：座位状态表格，选中空座后输入旅客信息完成售票
class SellDialog : public FadeDialog
{
    Q_OBJECT
public:
    explicit SellDialog(QWidget *parent = nullptr);

    void setTrain(Train *train);

signals:
    void dataChanged(); // 售票后数据变化

private:
    void onSell();

    SeatStatusTable *m_table;
    Train *m_train = nullptr;
};

#endif // SELLDIALOG_H
