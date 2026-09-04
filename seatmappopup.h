#ifndef SEATMAPPOPUP_H
#define SEATMAPPOPUP_H

#include <QWidget>
#include "train.h"

class QLabel;
class QPropertyAnimation;

// 座位周边浮窗：点击可选座位号时，展示该车厢座位分布（空座/已售/选中）
class SeatMapPopup : public QWidget
{
    Q_OBJECT
public:
    SeatMapPopup(const Train &train, int carriage, int selectedSeat, QWidget *parent = nullptr);

    void showNear(const QPoint &globalPos); // 在指定屏幕坐标附近弹出，带淡入动画

private:
    void buildGrid();

    Train m_train;
    int m_carriage;
    int m_selectedSeat;
    QPropertyAnimation *m_fadeIn = nullptr;
};

#endif // SEATMAPPOPUP_H
