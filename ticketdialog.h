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

private:
    Ui::TicketDialog *ui;
    Train *m_train;
};

#endif // TICKETDIALOG_H
