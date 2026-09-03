#include "seattableview.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

SeatTableView::SeatTableView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"车厢号", "座位号", "姓名", "身份证号"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_table);
}

void SeatTableView::setTrain(const Train *train)
{
    m_table->setRowCount(0);
    if (!train)
        return;
    const QVector<Seat> &seats = train->seats();
    int row = 0;
    for (const Seat &s : seats) {
        if (s.isEmpty())
            continue;
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(s.carriage())));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(s.seatNo())));
        m_table->setItem(row, 2, new QTableWidgetItem(s.name()));
        m_table->setItem(row, 3, new QTableWidgetItem(s.id()));
        ++row;
    }
}