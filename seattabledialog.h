#ifndef SEATTABLEDIALOG_H
#define SEATTABLEDIALOG_H

#include "fadedialog.h"
#include "train.h"

class SeatTableView;

// 座位登记弹窗：座位总览图 + 手动登记旅客
class SeatTableDialog : public FadeDialog
{
    Q_OBJECT
public:
    explicit SeatTableDialog(QWidget *parent = nullptr);

    void setTrain(Train *train); // nullptr 显示空状态

signals:
    void dataChanged(); // 登记后数据变化

private:
    SeatTableView *m_view;
};

#endif // SEATTABLEDIALOG_H
