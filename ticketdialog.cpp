#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include <QPushButton>
#include <QButtonGroup>
#include <QMessageBox>

TicketDialog::TicketDialog(Train *train, QWidget *parent)
    : FadeDialog(parent)
    , ui(new Ui::TicketDialog)
    , m_train(train)
{
    ui->setupUi(this);

    for (int c = 1; c <= train->carriages(); ++c)
        ui->carriageCombo->addItem(QString("%1号车厢 (%2)").arg(c).arg(train->carriageClassText(c)), c);

    // 分段切换：售票 / 退票
    auto *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addButton(ui->sellSeg);
    modeGroup->addButton(ui->refundSeg);
    connect(ui->sellSeg, &QPushButton::clicked, this, [this] { setSellMode(true); });
    connect(ui->refundSeg, &QPushButton::clicked, this, [this] { setSellMode(false); });

    connect(ui->carriageCombo, &QComboBox::currentIndexChanged, this, &TicketDialog::onCarriageChanged);

    rebuildSeatGrid();
}

TicketDialog::~TicketDialog()
{
    delete ui;
}

void TicketDialog::setSellMode(bool sell)
{
    const bool changed = (m_sell != sell);
    m_sell = sell;
    ui->sellSeg->setChecked(sell);
    ui->refundSeg->setChecked(!sell);
    ui->nameEdit->setEnabled(sell);
    ui->idEdit->setEnabled(sell);
    if (changed) {
        clearSelection();
        for (int i = 0; i < m_seatButtons.size(); ++i)
            styleSeatButton(m_seatButtons[i], i + 1);
        updateInfoLabel();
    }
}

bool TicketDialog::isSell() const { return m_sell; }
QString TicketDialog::name() const { return ui->nameEdit->text().trimmed(); }
QString TicketDialog::id() const { return ui->idEdit->text().trimmed(); }
int TicketDialog::carriage() const { return m_selectedCarriage; }
int TicketDialog::seatNo() const { return m_selectedSeat; }

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
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, seatNo);
    if (m_sell == occupied) // 售票只能点空座，退票只能点已售座
        return;

    m_selectedSeat = (seatNo == m_selectedSeat) ? 0 : seatNo;
    for (int i = 0; i < m_seatButtons.size(); ++i)
        styleSeatButton(m_seatButtons[i], i + 1);
    updateInfoLabel();
}

void TicketDialog::rebuildSeatGrid()
{
    // 清空旧座位按钮
    QLayoutItem *child;
    while ((child = ui->seatGridLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_seatButtons.clear();

    const int seatsPer = m_train->seatsPerCarriage();
    const int rows = (seatsPer + SeatsPerRow - 1) / SeatsPerRow;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < SeatsPerRow; ++c) {
            const int seatNo = r * SeatsPerRow + c + 1;
            if (seatNo > seatsPer)
                continue;
            auto *btn = new QPushButton(QString::number(seatNo), ui->seatArea);
            btn->setProperty("seat", true);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedSize(48, 36);
            ui->seatGridLayout->addWidget(btn, r, c, Qt::AlignCenter);
            m_seatButtons.append(btn);
            connect(btn, &QPushButton::clicked, this, [this, seatNo] { onSeatClicked(seatNo); });
            styleSeatButton(btn, seatNo);
        }
    }
    updateInfoLabel();
}

void TicketDialog::styleSeatButton(QPushButton *btn, int seatNo)
{
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, seatNo);
    const bool selected = (seatNo == m_selectedSeat);
    btn->setProperty("state", selected ? "selected" : (occupied ? "occupied" : "empty"));
    if (occupied)
        btn->setToolTip(QString("已售: %1")
            .arg(m_train->seats()[(m_selectedCarriage - 1) * m_train->seatsPerCarriage() + seatNo - 1].name()));
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
        ui->infoLabel->setText(QString("已选: %1号车厢 %2号座 (%3) · 票价 ¥%4")
                               .arg(m_selectedCarriage)
                               .arg(m_selectedSeat)
                               .arg(m_train->carriageClassText(m_selectedCarriage))
                               .arg(m_train->priceOf(m_selectedCarriage), 0, 'f', 2));
    } else {
        ui->infoLabel->setText(m_sell ? "请点击绿色空座进行选择" : "请点击红色已售座位进行退票");
    }
}

void TicketDialog::accept()
{
    if (m_selectedSeat <= 0) {
        QMessageBox::warning(this, "提示", m_sell ? "请先选择一个空座" : "请先选择要退票的座位");
        return;
    }
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, m_selectedSeat);
    if (m_sell) {
        if (name().isEmpty() || id().isEmpty()) {
            QMessageBox::warning(this, "提示", "姓名和身份证号不能为空");
            return;
        }
        if (!Train::isValidId(id())) {
            QMessageBox::warning(this, "提示", "身份证号格式不正确（18位）");
            return;
        }
        if (occupied) {
            QMessageBox::warning(this, "提示", "该座位已被占用");
            return;
        }
    } else {
        if (!occupied) {
            QMessageBox::warning(this, "提示", "该座位没有旅客，无法退票");
            return;
        }
    }
    QDialog::accept();
}
