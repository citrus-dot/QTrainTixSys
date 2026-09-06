#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include "registerdialog.h"
#include "ticketview.h"
#include "tableutil.h"
#include <QPushButton>
#include <QMessageBox>
#include <QStyle>

TicketDialog::TicketDialog(Train *train, QWidget *parent)
    : FadeDialog(parent)
    , ui(new Ui::TicketDialog)
    , m_train(train)
{
    ui->setupUi(this);

    for (int c = 1; c <= train->carriages(); ++c)
        ui->carriageCombo->addItem(QString("%1号车厢 (%2)").arg(c).arg(train->carriageClassText(c)), c);

    connect(ui->carriageCombo, &QComboBox::currentIndexChanged, this, &TicketDialog::onCarriageChanged);
    connect(ui->actionButton, &QPushButton::clicked, this, &TicketDialog::onAction);

    rebuildSeatGrid();
}

TicketDialog::~TicketDialog()
{
    delete ui;
}

void TicketDialog::onCarriageChanged()
{
    m_selectedCarriage = ui->carriageCombo->currentData().toInt();
    clearSelection();
    rebuildSeatGrid();
}

void TicketDialog::onSeatClicked(int seatNo)
{
    if (seatNo > m_train->seatsPerCarriage())
        return;
    m_selectedSeat = (seatNo == m_selectedSeat) ? 0 : seatNo;
    for (int i = 0; i < m_seatButtons.size(); ++i)
        styleSeatButton(m_seatButtons[i], i + 1);
    updateInfoLabel();
    updateActionButton();
}

void TicketDialog::rebuildSeatGrid()
{
    // 清空旧座位按钮
    clearLayout(ui->seatGridLayout);
    m_seatButtons.clear();

    const int seatsPer = m_train->seatsPerCarriage();
    const int rows = (seatsPer + SeatsPerRow - 1) / SeatsPerRow;
    const int aisleAfter = rows - 2; // 过道插在最后两排之间（2排→正中，3排→二/三排间）
    // 座位图节奏：座位块高 36；过道与座位间距 = 座位高一半(18)；过道带高 = 座位块高(36)；
    // 图上下留白比座位块高度略短一点(30)
    const int seatH = 36;
    const int gap = seatH / 2;
    const int outer = seatH - 6;
    ui->seatGridLayout->setVerticalSpacing(gap);
    ui->seatGridLayout->setContentsMargins(0, outer, 0, outer);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < SeatsPerRow; ++c) {
            const int seatNo = r * SeatsPerRow + c + 1;
            if (seatNo > seatsPer)
                continue;
            auto *btn = new QPushButton(QString::number(seatNo), ui->seatArea);
            btn->setProperty("seat", true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(48, seatH);
            ui->seatGridLayout->addWidget(btn, seatGridRow(r, aisleAfter), c, Qt::AlignCenter);
            m_seatButtons.append(btn);
            connect(btn, &QPushButton::clicked, this, [this, seatNo] { onSeatClicked(seatNo); });
            styleSeatButton(btn, seatNo);
        }
    }
    addAisleRow(ui->seatGridLayout, rows, SeatsPerRow, ui->seatArea, seatH);

    // 按网格实际高度收紧滚动区与窗口：排间距 rows 处 + 过道带，不多留空
    const int gridH = rows * seatH + 2 * outer
        + (rows >= 2 ? seatH + rows * gap : 0);
    ui->seatScroll->setFixedHeight(gridH);
    setFixedHeight(sizeHint().height());
    updateInfoLabel();
    updateActionButton();
}

void TicketDialog::styleSeatButton(QPushButton *btn, int seatNo)
{
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, seatNo);
    const bool selected = (seatNo == m_selectedSeat);
    btn->setProperty("state", selected ? "selected" : (occupied ? "occupied" : "empty"));
    btn->setProperty("class", m_train->carriageClass(m_selectedCarriage) == 1 ? "first" : "second");
    if (occupied)
        btn->setToolTip(QString("已售: %1")
            .arg(m_train->seatAt(m_selectedCarriage, seatNo).name()));
    else
        btn->setToolTip(QString("%1号座").arg(seatNo));
    btn->style()->unpolish(btn);
    btn->style()->polish(btn);
}

void TicketDialog::clearSelection()
{
    m_selectedSeat = 0;
}

void TicketDialog::updateInfoLabel()
{
    if (m_selectedSeat > 0) {
        const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, m_selectedSeat);
        if (occupied) {
            const Seat &s = m_train->seatAt(m_selectedCarriage, m_selectedSeat);
            ui->infoLabel->setText(QString("已选: %1号车厢 %2号座 · 已售给 %3")
                                   .arg(m_selectedCarriage).arg(m_selectedSeat).arg(s.name()));
        } else {
            ui->infoLabel->setText(QString("已选: %1号车厢 %2号座 (%3) · 票价 ¥%4")
                                   .arg(m_selectedCarriage)
                                   .arg(m_selectedSeat)
                                   .arg(m_train->carriageClassText(m_selectedCarriage))
                                   .arg(m_train->priceOf(m_selectedCarriage), 0, 'f', 2));
        }
    } else {
        ui->infoLabel->setText("请点击座位进行选择");
    }
}

void TicketDialog::updateActionButton()
{
    if (m_selectedSeat <= 0) {
        ui->actionButton->setEnabled(false);
        ui->actionButton->setText("请先选择座位");
        return;
    }
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, m_selectedSeat);
    ui->actionButton->setEnabled(true);
    ui->actionButton->setText(occupied ? "退票" : "售票");
}

void TicketDialog::onAction()
{
    if (m_selectedSeat <= 0)
        return;
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, m_selectedSeat);
    if (occupied) {
        if (QMessageBox::question(this, "确认退票",
                                  QString("确定退掉 %1号车厢 %2号座的票吗？").arg(m_selectedCarriage).arg(m_selectedSeat))
            != QMessageBox::Yes)
            return;
        if (!m_train->refundTicket(m_selectedCarriage, m_selectedSeat))
            return;
        clearSelection();
        rebuildSeatGrid();
        emit dataChanged();
    } else {
        RegisterDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted)
            return;
        const int savedCarriage = m_selectedCarriage;
        const int savedSeat = m_selectedSeat;
        if (!m_train->sellTicket(dlg.name(), dlg.id(), savedCarriage, savedSeat)) {
            QMessageBox::warning(this, "提示", "售票失败，座位可能已被占用");
            return;
        }
        clearSelection();
        rebuildSeatGrid();
        emit dataChanged();
        TicketView view(m_train, dlg.name(), dlg.id(), savedCarriage, savedSeat, this);
        view.exec();
    }
}
