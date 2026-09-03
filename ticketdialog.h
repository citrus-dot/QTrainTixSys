#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include "fadedialog.h"
#include "train.h"
#include <QVector>

class QPushButton;

namespace Ui { class TicketDialog; }

class TicketDialog : public FadeDialog
{
    Q_OBJECT

public:
    explicit TicketDialog(Train *train, QWidget *parent = nullptr);
    ~TicketDialog() override;

    void setSellMode(bool sell);
    bool isSell() const;
    QString name() const;
    QString id() const;
    int carriage() const;
    int seatNo() const;

protected:
    void accept() override;

private slots:
    void onCarriageChanged();
    void onSeatClicked(int seatNo);

private:
    void rebuildSeatGrid();
    void styleSeatButton(QPushButton *btn, int seatNo);
    void clearSelection();
    void updateInfoLabel();

    Ui::TicketDialog *ui;
    Train *m_train;
    bool m_sell = true;
    int m_selectedCarriage = 1;
    int m_selectedSeat = 0;
    QVector<QPushButton *> m_seatButtons;
    static constexpr int SeatsPerRow = 5;
};

#endif // TICKETDIALOG_H
