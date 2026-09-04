#include "selldialog.h"
#include "seatstatustable.h"
#include "registerdialog.h"
#include "ticketview.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

SellDialog::SellDialog(QWidget *parent)
    : FadeDialog(parent)
{
    setWindowTitle("售票");
    setMinimumSize(660, 480);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(12);

    m_table = new SeatStatusTable(this);
    m_table->setSellMode(true);
    layout->addWidget(m_table, 1);

    auto *btn = new QPushButton("确认售票", this);
    btn->setEnabled(false);
    layout->addWidget(btn);

    connect(m_table, &SeatStatusTable::selectionChanged, this, [this, btn] {
        btn->setEnabled(m_table->selectedSeat() > 0);
    });
    connect(btn, &QPushButton::clicked, this, &SellDialog::onSell);
}

void SellDialog::setTrain(Train *train)
{
    m_train = train;
    m_table->setTrain(train);
}

void SellDialog::onSell()
{
    if (!m_train)
        return;
    const int carriage = m_table->selectedCarriage();
    const int seatNo = m_table->selectedSeat();
    if (carriage <= 0 || seatNo <= 0)
        return;
    RegisterDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (!m_train->sellTicket(dlg.name(), dlg.id(), carriage, seatNo)) {
        QMessageBox::warning(this, "提示", "售票失败，座位可能已被占用");
        return;
    }
    m_table->setTrain(m_train);
    emit dataChanged();
    TicketView view(m_train, dlg.name(), dlg.id(), carriage, seatNo, this);
    view.exec();
}
