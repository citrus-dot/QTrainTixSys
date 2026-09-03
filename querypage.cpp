#include "querypage.h"
#include "ui_querypage.h"
#include "seatmappopup.h"
#include <QListWidgetItem>
#include <QToolTip>
#include <QCursor>

QueryPage::QueryPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QueryPage)
{
    ui->setupUi(this);
    connect(ui->comboTrain, &QComboBox::currentIndexChanged, this, &QueryPage::onTrainChanged);
    connect(ui->seatList, &QListWidget::itemClicked, this, &QueryPage::onSeatClicked);
    connect(ui->stopList, &QListWidget::itemClicked, this, &QueryPage::onStopClicked);
}

QueryPage::~QueryPage()
{
    delete ui;
}

void QueryPage::setSystem(const TrainSystem *system)
{
    m_system = system;
}

void QueryPage::refresh()
{
    const QString prev = ui->comboTrain->currentText();
    ui->comboTrain->blockSignals(true);
    ui->comboTrain->clear();
    if (m_system) {
        for (const Train &t : m_system->trains())
            ui->comboTrain->addItem(t.no());
    }
    const int idx = ui->comboTrain->findText(prev);
    ui->comboTrain->setCurrentIndex(idx >= 0 ? idx : 0);
    ui->comboTrain->blockSignals(false);
    onTrainChanged();
}

void QueryPage::onTrainChanged()
{
    const bool hasData = m_system && !m_system->trains().isEmpty()
                         && ui->comboTrain->currentIndex() >= 0
                         && ui->comboTrain->currentIndex() < m_system->trains().size();
    if (!hasData) {
        ui->dateValue->setText("-");
        ui->timeValue->setText("-");
        ui->fromValue->setText("-");
        ui->toValue->setText("-");
        ui->remainingValue->setText("-");
        ui->firstPriceValue->setText("-");
        ui->secondPriceValue->setText("-");
        ui->firstClassValue->setText("-");
        ui->seatList->clear();
        ui->stopList->clear();
        return;
    }

    const Train &t = m_system->trains()[ui->comboTrain->currentIndex()];
    ui->dateValue->setText(t.date());
    ui->timeValue->setText(t.departTime());
    ui->fromValue->setText(t.from());
    ui->toValue->setText(t.to());
    ui->remainingValue->setText(QString::number(t.remainingSeats()));
    ui->firstPriceValue->setText(QString("¥ %1").arg(t.firstClassPrice(), 0, 'f', 2));
    ui->secondPriceValue->setText(QString("¥ %1").arg(t.secondClassPrice(), 0, 'f', 2));

    QStringList firstClassList;
    for (int c = 1; c <= t.carriages(); ++c)
        if (t.carriageClass(c) == 1)
            firstClassList << QString::number(c);
    ui->firstClassValue->setText(firstClassList.isEmpty() ? "无" : firstClassList.join(",") + "号车厢");

    ui->seatList->clear();
    for (int c = 1; c <= t.carriages(); ++c) {
        const QVector<int> seats = t.availableSeats(c);
        for (int s : seats) {
            auto *item = new QListWidgetItem(QString("%1号车厢 %2号座").arg(c).arg(s));
            item->setData(Qt::UserRole, c);
            item->setData(Qt::UserRole + 1, s);
            ui->seatList->addItem(item);
        }
    }

    ui->stopList->clear();
    int idx = 0;
    for (const QString &s : t.stops()) {
        auto *item = new QListWidgetItem(s);
        item->setData(Qt::UserRole, idx);
        ui->stopList->addItem(item);
        ++idx;
    }
}

void QueryPage::onSeatClicked(QListWidgetItem *item)
{
    if (!m_system || m_system->trains().isEmpty())
        return;
    const int carriage = item->data(Qt::UserRole).toInt();
    const int seatNo = item->data(Qt::UserRole + 1).toInt();
    const Train &t = m_system->trains()[ui->comboTrain->currentIndex()];

    if (m_seatPopup) {
        m_seatPopup->deleteLater();
        m_seatPopup = nullptr;
    }
    m_seatPopup = new SeatMapPopup(t, carriage, seatNo, this);
    m_seatPopup->showNear(QCursor::pos());
}

void QueryPage::onStopClicked(QListWidgetItem *item)
{
    const int idx = item->data(Qt::UserRole).toInt();
    QToolTip::showText(QCursor::pos(),
                       QString("第 %1 站 · 途经站").arg(idx + 1),
                       ui->stopList);
}
