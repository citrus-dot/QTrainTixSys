#include "seatmappopup.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QStyle>

SeatMapPopup::SeatMapPopup(const Train &train, int carriage, int selectedSeat, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_train(train)
    , m_carriage(carriage)
    , m_selectedSeat(selectedSeat)
{
    setObjectName("seatMapPopup");
    setAttribute(Qt::WA_StyledBackground, true);
    buildGrid();
}

void SeatMapPopup::buildGrid()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    auto *title = new QLabel(QString("%1号车厢 · %2 · 座位分布")
                                 .arg(m_carriage)
                                 .arg(m_train.carriageClassText(m_carriage)), this);
    title->setObjectName("popupTitle");
    layout->addWidget(title);

    auto *grid = new QGridLayout;
    grid->setSpacing(6);
    const int seatsPer = m_train.seatsPerCarriage();
    const int cols = 5;
    for (int i = 0; i < seatsPer; ++i) {
        const int seatNo = i + 1;
        auto *cell = new QLabel(QString::number(seatNo), this);
        cell->setObjectName("seatCell");
        cell->setAlignment(Qt::AlignCenter);
        cell->setFixedSize(34, 26);
        const bool selected = (seatNo == m_selectedSeat);
        const bool occupied = m_train.isSeatOccupied(m_carriage, seatNo);
        cell->setProperty("state", selected ? "selected" : (occupied ? "occupied" : "empty"));
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
        grid->addWidget(cell, i / cols, i % cols);
    }
    layout->addLayout(grid);

    auto *legend = new QLabel(
        "<span style='color:#1E7B45'>■ 空座</span>"
        "&nbsp;&nbsp;<span style='color:#C0392B'>■ 已售</span>"
        "&nbsp;&nbsp;<span style='color:#2F6FED'>■ 选中</span>", this);
    legend->setObjectName("popupLegend");
    layout->addWidget(legend);
}

void SeatMapPopup::showNear(const QPoint &globalPos)
{
    adjustSize();
    QScreen *screen = QGuiApplication::screenAt(globalPos);
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    const QRect avail = screen->availableGeometry();
    int x = globalPos.x() + 12;
    int y = globalPos.y() + 12;
    if (x + width() > avail.right())
        x = globalPos.x() - width() - 12;
    if (y + height() > avail.bottom())
        y = globalPos.y() - height() - 12;
    move(qMax(x, avail.left() + 4), qMax(y, avail.top() + 4));
    show();
}
