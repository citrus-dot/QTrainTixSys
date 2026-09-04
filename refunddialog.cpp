#include "refunddialog.h"
#include "seatstatustable.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

RefundDialog::RefundDialog(QWidget *parent)
    : FadeDialog(parent)
{
    setWindowTitle("退票");
    setMinimumSize(660, 480);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(12);

    m_table = new SeatStatusTable(this);
    m_table->setSellMode(false);
    layout->addWidget(m_table, 1);

    auto *btn = new QPushButton("确认退票", this);
    btn->setEnabled(false);
    layout->addWidget(btn);

    connect(m_table, &SeatStatusTable::selectionChanged, this, [this, btn] {
        btn->setEnabled(m_table->selectedSeat() > 0);
    });
    connect(btn, &QPushButton::clicked, this, &RefundDialog::onRefund);
}

void RefundDialog::setTrain(Train *train)
{
    m_train = train;
    m_table->setTrain(train);
}

void RefundDialog::onRefund()
{
    if (!m_train)
        return;
    const int carriage = m_table->selectedCarriage();
    const int seatNo = m_table->selectedSeat();
    if (carriage <= 0 || seatNo <= 0)
        return;
    if (QMessageBox::question(this, "确认退票",
                              QString("确定退掉 %1号车厢 %2号座的票吗？").arg(carriage).arg(seatNo))
        != QMessageBox::Yes)
        return;
    if (!m_train->refundTicket(carriage, seatNo)) {
        QMessageBox::warning(this, "提示", "退票失败");
        return;
    }
    m_table->setTrain(m_train);
    emit dataChanged();
    QMessageBox::information(this, "提示", "退票成功");
}
