#ifndef SEATTABLEVIEW_H
#define SEATTABLEVIEW_H

#include <QWidget>
#include "train.h"

class QLabel;
class QComboBox;
class QGridLayout;
class QPushButton;
class QFrame;
class QStackedLayout;

// 座位登记视图：车厢座位总览图 + 点击空座手动登记旅客
class SeatTableView : public QWidget
{
    Q_OBJECT
public:
    explicit SeatTableView(QWidget *parent = nullptr);

    void setTrain(Train *train); // nullptr 清空

signals:
    void dataChanged(); // 登记后数据变化，通知外部刷新

private:
    void rebuildGrid();
    void onSeatClicked(int seatNo);
    void styleSeatButton(QPushButton *btn, int seatNo);
    void updateStats();

    Train *m_train = nullptr;
    int m_carriage = 1;
    QLabel *m_title;
    QComboBox *m_carriageCombo;
    QWidget *m_gridContainer;
    QGridLayout *m_grid;
    QVector<QPushButton *> m_seatButtons;
    QLabel *m_statsLabel;
    QFrame *m_emptyFrame;
    QLabel *m_emptyHint;
    QStackedLayout *m_stack;
};

#endif // SEATTABLEVIEW_H
