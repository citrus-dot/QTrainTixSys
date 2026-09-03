#include "seattableview.h"
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

SeatTableView::SeatTableView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_title = new QLabel("座位登记", this);
    m_title->setObjectName("panelTitle");
    layout->addWidget(m_title);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"车厢号", "座位号", "姓名", "身份证号"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table, 1);

    m_emptyHint = new QLabel("请先在左侧选择一个班次\n查看该班次已售座位", this);
    m_emptyHint->setObjectName("emptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    layout->addWidget(m_emptyHint);
}

void SeatTableView::setTrain(const Train *train)
{
    m_table->setRowCount(0);
    if (!train) {
        m_table->setVisible(false);
        m_emptyHint->setVisible(true);
        m_emptyHint->setText("请先在左侧选择一个班次\n查看该班次已售座位");
        return;
    }
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
    const bool empty = m_table->rowCount() == 0;
    m_table->setVisible(!empty);
    m_emptyHint->setVisible(empty);
    if (empty)
        m_emptyHint->setText("该班次暂无已售座位\n点击「售票」开始售票");
}
