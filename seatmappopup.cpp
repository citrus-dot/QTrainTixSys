#include "seatmappopup.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QStyle>
#include <QPropertyAnimation>
#include <QFrame>

SeatMapPopup::SeatMapPopup(const Train &train, int carriage, int selectedSeat, QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , m_train(train)
    , m_carriage(carriage)
    , m_selectedSeat(selectedSeat)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    // 外层透明窗口：无黑色边框

    // 内容容器：实心白色背景 + 边框，为文字提供不透背景
    auto *content = new QFrame(this);
    content->setObjectName("seatMapPopup");
    content->setAutoFillBackground(true);
    content->setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    auto *title = new QLabel(QString("%1号车厢 · %2 · 座位分布")
                                 .arg(m_carriage)
                                 .arg(m_train.carriageClassText(m_carriage)), content);
    title->setObjectName("popupTitle");
    layout->addWidget(title);

    auto *grid = new QGridLayout;
    grid->setSpacing(6);
    const int seatsPer = m_train.seatsPerCarriage();
    const int cols = 5;
    for (int i = 0; i < seatsPer; ++i) {
        const int seatNo = i + 1;
        auto *cell = new QLabel(QString::number(seatNo), content);
        cell->setObjectName("seatCell");
        cell->setAlignment(Qt::AlignCenter);
        cell->setFixedSize(34, 26);
        const bool occupied = m_train.isSeatOccupied(m_carriage, seatNo);
        const bool selected = (seatNo == m_selectedSeat);
        if (selected)
            cell->setProperty("state", "selected");
        else
            cell->setProperty("state", occupied ? "occupied" : "empty");
        cell->style()->unpolish(cell);
        cell->style()->polish(cell);
        grid->addWidget(cell, i / cols, i % cols);
    }
    layout->addLayout(grid);

    auto *legend = new QLabel(
        "<span style='color:#1E7B45'>■ 空座</span>"
        "&nbsp;&nbsp;<span style='color:#C0392B'>■ 已售</span>"
        "&nbsp;&nbsp;<span style='color:#2F6FED'>■ 选中</span>", content);
    legend->setObjectName("popupLegend");
    layout->addWidget(legend);

    // 将内容容器作为唯一子控件，填充整个弹出窗口
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(content);
    setLayout(outerLayout);
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

    // 淡入动画（DeleteWhenStopped 自动释放）
    setWindowOpacity(0.0);
    show();
    auto *fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(150);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}
