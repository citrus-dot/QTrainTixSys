#ifndef TICKETVIEW_H
#define TICKETVIEW_H

#include "fadedialog.h"
#include "train.h"

namespace Ui { class TicketView; }

class TicketView : public FadeDialog
{
    Q_OBJECT

public:
    explicit TicketView(const Train *train, const QString &name, const QString &id,
                        int carriage, int seatNo, QWidget *parent = nullptr);
    ~TicketView() override;

private slots:
    void onSaveTicket();

private:
    Ui::TicketView *ui;
    const Train *m_train;
    QString m_name;
    QString m_id;
    int m_carriage;
    int m_seatNo;
};

#endif // TICKETVIEW_H
