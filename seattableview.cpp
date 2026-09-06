#include "seattableview.h"
#include "registerdialog.h"
#include "tableutil.h"
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QFrame>
#include <QToolTip>
#include <QCursor>
#include <QMessageBox>
#include <QStyle>

SeatTableView::SeatTableView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_title = new QLabel("座位登记", this);
    m_title->setObjectName("panelTitle");
    layout->addWidget(m_title);

    // 顶部：车厢选择 + 图例
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(10);
    auto *carriageLabel = new QLabel("车厢", this);
    m_carriageCombo = new QComboBox(this);
    m_carriageCombo->setMinimumWidth(150);
    topRow->addWidget(carriageLabel);
    topRow->addWidget(m_carriageCombo);
    topRow->addStretch();
    auto *legend = new QLabel(
        "<span style='color:#1E7B45'>■ 空座</span>"
        "&nbsp;&nbsp;<span style='color:#C0392B'>■ 已售</span>", this);
    legend->setObjectName("popupLegend");
    topRow->addWidget(legend);
    layout->addLayout(topRow);

    // 座位网格容器
    m_gridContainer = new QWidget(this);
    m_grid = new QGridLayout(m_gridContainer);
    m_grid->setSpacing(8);
    m_grid->setContentsMargins(0, 0, 0, 0);

    // 空状态引导：置于网格框内
    m_emptyFrame = new QFrame(this);
    m_emptyFrame->setObjectName("emptyFrame");
    auto *emptyLayout = new QVBoxLayout(m_emptyFrame);
    m_emptyHint = new QLabel(m_emptyFrame);
    m_emptyHint->setObjectName("emptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    emptyLayout->addWidget(m_emptyHint);

    m_stack = new QStackedLayout;
    m_stack->addWidget(m_gridContainer);
    m_stack->addWidget(m_emptyFrame);
    layout->addLayout(m_stack, 1);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName("popupLegend");
    layout->addWidget(m_statsLabel);

    connect(m_carriageCombo, &QComboBox::currentIndexChanged, this, [this] {
        m_carriage = m_carriageCombo->currentData().toInt();
        rebuildGrid();
    });
}

void SeatTableView::setTrain(Train *train)
{
    m_train = train;
    m_carriageCombo->blockSignals(true);
    m_carriageCombo->clear();
    if (train) {
        for (int c = 1; c <= train->carriages(); ++c)
            m_carriageCombo->addItem(QString("%1号车厢 (%2)").arg(c).arg(train->carriageClassText(c)), c);
        m_carriage = 1;
        m_carriageCombo->setCurrentIndex(0);
    }
    m_carriageCombo->blockSignals(false);
    rebuildGrid();
}

void SeatTableView::rebuildGrid()
{
    // 清空旧座位按钮
    clearLayout(m_grid);
    m_seatButtons.clear();

    if (!m_train) {
        m_stack->setCurrentWidget(m_emptyFrame);
        m_emptyHint->setText("请先在左侧选择一个班次\n查看座位分布或登记旅客");
        m_statsLabel->clear();
        return;
    }

    const int seatsPer = m_train->seatsPerCarriage();
    const int cols = 8;
    const int rows = (seatsPer + cols - 1) / cols;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int seatNo = r * cols + c + 1;
            if (seatNo > seatsPer)
                continue;
            auto *btn = new QPushButton(QString::number(seatNo), m_gridContainer);
            btn->setProperty("seat", true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(52, 34);
            m_grid->addWidget(btn, r, c, Qt::AlignCenter);
            m_seatButtons.append(btn);
            connect(btn, &QPushButton::clicked, this, [this, seatNo] { onSeatClicked(seatNo); });
            styleSeatButton(btn, seatNo);
        }
    }
    m_stack->setCurrentWidget(m_seatButtons.isEmpty() ? m_emptyFrame : m_gridContainer);
    updateStats();
}

void SeatTableView::onSeatClicked(int seatNo)
{
    if (!m_train)
        return;
    if (m_train->isSeatOccupied(m_carriage, seatNo)) {
        const Seat &s = m_train->seats()[(m_carriage - 1) * m_train->seatsPerCarriage() + seatNo - 1];
        QToolTip::showText(QCursor::pos(),
                           QString("已售 · %1号车厢 %2号座\n旅客: %3\n身份证: %4")
                               .arg(m_carriage).arg(seatNo).arg(s.name()).arg(maskId(s.id())),
                           this);
        return;
    }
    // 空座：弹出登记对话框
    RegisterDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (m_train->sellTicket(dlg.name(), dlg.id(), m_carriage, seatNo)) {
        rebuildGrid();
        emit dataChanged();
    } else {
        QMessageBox::warning(this, "提示", "登记失败，座位可能已被占用");
    }
}

void SeatTableView::styleSeatButton(QPushButton *btn, int seatNo)
{
    const bool occupied = m_train->isSeatOccupied(m_carriage, seatNo);
    btn->setProperty("state", occupied ? "occupied" : "empty");
    if (occupied) {
        const Seat &s = m_train->seatAt(m_carriage, seatNo);
        btn->setToolTip(QString("已售: %1").arg(s.name()));
    } else {
        btn->setToolTip(QString("%1号座 · 点击登记").arg(seatNo));
    }
    btn->style()->unpolish(btn);
    btn->style()->polish(btn);
}

void SeatTableView::updateStats()
{
    if (!m_train) {
        m_statsLabel->clear();
        return;
    }
    int sold = 0;
    for (int s = 1; s <= m_train->seatsPerCarriage(); ++s)
        if (m_train->isSeatOccupied(m_carriage, s))
            ++sold;
    m_statsLabel->setText(QString("本车厢已售 %1 / %2 座 · 点击空座可登记旅客")
                          .arg(sold).arg(m_train->seatsPerCarriage()));
}
