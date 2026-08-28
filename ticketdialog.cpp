#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QBrush>
#include <QColor>

TicketDialog::TicketDialog(Train *train, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TicketDialog)
    , m_train(train)
{
    ui->setupUi(this);

    for (int c = 1; c <= train->carriages(); ++c)
        ui->carriageCombo->addItem(QString("%1号车厢 (%2)").arg(c).arg(train->carriageClassText(c)), c);

    connect(ui->carriageCombo, &QComboBox::currentIndexChanged, this, &TicketDialog::onCarriageChanged);
    connect(ui->seatGrid, &QTableWidget::cellClicked, this, &TicketDialog::onSeatClicked);
    connect(ui->sellRadio, &QRadioButton::toggled, this, [this](bool checked) { setSellMode(checked); });
    connect(ui->refundRadio, &QRadioButton::toggled, this, [this](bool checked) { setSellMode(!checked); });

    ui->seatGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->seatGrid->verticalHeader()->setDefaultSectionSize(30);

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
    ui->sellRadio->setChecked(sell);
    ui->refundRadio->setChecked(!sell);
    ui->nameEdit->setEnabled(sell);
    ui->idEdit->setEnabled(sell);
    if (changed) {
        clearSelection();
        rebuildSeatGrid();
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

void TicketDialog::onSeatClicked(int row, int col)
{
    const int seatNo = row * SeatsPerRow + col + 1;
    if (seatNo > m_train->seatsPerCarriage())
        return;
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, seatNo);
    if (m_sell == occupied) // 售票只能点空座，退票只能点已售座
        return;

    const int oldRow = m_selectedRow, oldCol = m_selectedCol;
    if (row == oldRow && col == oldCol) {
        clearSelection();
    } else {
        m_selectedRow = row;
        m_selectedCol = col;
        m_selectedSeat = seatNo;
    }
    if (oldRow >= 0)
        styleSeatItem(oldRow, oldCol);
    styleSeatItem(row, col);
    updateInfoLabel();
}

void TicketDialog::rebuildSeatGrid()
{
    const int seatsPer = m_train->seatsPerCarriage();
    const int rows = (seatsPer + SeatsPerRow - 1) / SeatsPerRow;
    ui->seatGrid->clear();
    ui->seatGrid->setRowCount(rows);
    ui->seatGrid->setColumnCount(SeatsPerRow);
    for (int c = 1; c <= SeatsPerRow; ++c)
        ui->seatGrid->setHorizontalHeaderItem(c - 1, new QTableWidgetItem(QString::number(c)));

    for (int r = 0; r < rows; ++r) {
        ui->seatGrid->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
        for (int c = 0; c < SeatsPerRow; ++c) {
            const int seatNo = r * SeatsPerRow + c + 1;
            QTableWidgetItem *item;
            if (seatNo > seatsPer) {
                item = new QTableWidgetItem();
                item->setFlags(Qt::NoItemFlags);
            } else {
                item = new QTableWidgetItem(QString::number(seatNo));
                item->setTextAlignment(Qt::AlignCenter);
                if (m_train->isSeatOccupied(m_selectedCarriage, seatNo))
                    item->setToolTip(QString("已售: %1")
                        .arg(m_train->seats()[(m_selectedCarriage - 1) * seatsPer + seatNo - 1].name()));
            }
            ui->seatGrid->setItem(r, c, item);
            styleSeatItem(r, c);
        }
    }
    updateInfoLabel();
}

void TicketDialog::styleSeatItem(int row, int col)
{
    QTableWidgetItem *item = ui->seatGrid->item(row, col);
    if (!item)
        return;
    const int seatNo = row * SeatsPerRow + col + 1;
    if (seatNo > m_train->seatsPerCarriage())
        return;
    const bool occupied = m_train->isSeatOccupied(m_selectedCarriage, seatNo);
    const bool selected = (row == m_selectedRow && col == m_selectedCol);
    if (selected) {
        item->setBackground(QBrush(QColor(255, 170, 60))); // 橙色高亮
        item->setForeground(QBrush(Qt::black));
    } else if (occupied) {
        item->setBackground(QBrush(QColor(220, 70, 70))); // 红色=已售
        item->setForeground(QBrush(Qt::white));
    } else {
        item->setBackground(QBrush(QColor(120, 200, 120))); // 绿色=空座
        item->setForeground(QBrush(Qt::white));
    }
}

void TicketDialog::clearSelection()
{
    m_selectedSeat = 0;
    m_selectedRow = -1;
    m_selectedCol = -1;
}

void TicketDialog::updateInfoLabel()
{
    if (m_selectedSeat > 0) {
        ui->infoLabel->setText(QString("已选: %1号车厢 %2号座 (%3)")
                               .arg(m_selectedCarriage)
                               .arg(m_selectedSeat)
                               .arg(m_train->carriageClassText(m_selectedCarriage)));
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
