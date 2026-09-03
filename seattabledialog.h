#ifndef SEATTABLEDIALOG_H
#define SEATTABLEDIALOG_H

#include "fadedialog.h"
#include "train.h"

class SeatTableView;

// 座位登记弹窗：淡入展示当前班次已售座位列表
class SeatTableDialog : public FadeDialog
{
    Q_OBJECT
public:
    explicit SeatTableDialog(QWidget *parent = nullptr);

    void setTrain(const Train *train); // nullptr 显示空状态

private:
    SeatTableView *m_view;
};

#endif // SEATTABLEDIALOG_H
