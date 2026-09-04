#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include "fadedialog.h"
#include "train.h"
#include <QVector>

class QPushButton;

namespace Ui { class TicketDialog; }

// 班次详情弹窗：座位网格总览，选中座位后在下方给出可进行的操作
class TicketDialog : public FadeDialog
{
    Q_OBJECT

public:
    explicit TicketDialog(Train *train, QWidget *parent = nullptr);
    ~TicketDialog() override;

signals:
    void dataChanged(); // 售票/退票后数据变化

private slots:
    void onCarriageChanged();
    void onSeatClicked(int seatNo);

private:
    void rebuildSeatGrid();
    void styleSeatButton(QPushButton *btn, int seatNo);
    void clearSelection();
    void updateInfoLabel();
    void updateActionButton();
    void onAction();

    Ui::TicketDialog *ui;
    Train *m_train;
    int m_selectedCarriage = 1;
    int m_selectedSeat = 0;
    QVector<QPushButton *> m_seatButtons;
    static constexpr int SeatsPerRow = 5;
};

#endif // TICKETDIALOG_H
