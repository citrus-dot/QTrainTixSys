#ifndef TICKETDIALOG_H
#define TICKETDIALOG_H

#include <QDialog>
#include "train.h"

namespace Ui { class TicketDialog; }

class TicketDialog : public QDialog
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
    void onSeatClicked(int row, int col);

private:
    void rebuildSeatGrid();
    void styleSeatItem(int row, int col);
    void clearSelection();
    void updateInfoLabel();

    Ui::TicketDialog *ui;
    Train *m_train;
    bool m_sell = true;
    int m_selectedCarriage = 1;
    int m_selectedSeat = 0;
    int m_selectedRow = -1;
    int m_selectedCol = -1;
    static constexpr int SeatsPerRow = 5;
};

#endif // TICKETDIALOG_H
