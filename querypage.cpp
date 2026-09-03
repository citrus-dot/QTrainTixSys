#include "querypage.h"
#include "ui_querypage.h"

QueryPage::QueryPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QueryPage)
{
    ui->setupUi(this);
    connect(ui->comboTrain, &QComboBox::currentIndexChanged, this, &QueryPage::onTrainChanged);
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
    if (!m_system || m_system->trains().isEmpty()) {
        ui->infoLabel->setText("暂无班次数据\n请先在「班次管理」页新增班次或打开数据文件");
        ui->priceLabel->clear();
        ui->seatList->clear();
        ui->stopList->clear();
        return;
    }
    const int index = ui->comboTrain->currentIndex();
    if (index < 0 || index >= m_system->trains().size()) {
        ui->infoLabel->setText("暂无班次数据\n请先在「班次管理」页新增班次或打开数据文件");
        ui->priceLabel->clear();
        ui->seatList->clear();
        ui->stopList->clear();
        return;
    }
    const Train &t = m_system->trains()[index];
    ui->infoLabel->setText(QString("日期: %1    发车时间: %2    发车城市: %3    终点城市: %4    余票数: %5")
                           .arg(t.date(), t.departTime(), t.from(), t.to())
                           .arg(t.remainingSeats()));

    QStringList firstClassList;
    for (int c = 1; c <= t.carriages(); ++c)
        if (t.carriageClass(c) == 1)
            firstClassList << QString::number(c);
    const QString classInfo = firstClassList.isEmpty()
        ? "一等座: 无"
        : "一等座: " + firstClassList.join(",") + "号车厢";
    ui->priceLabel->setText(QString("一等票价: %1 元    二等票价: %2 元    %3")
                            .arg(t.firstClassPrice(), 0, 'f', 2)
                            .arg(t.secondClassPrice(), 0, 'f', 2)
                            .arg(classInfo));

    ui->seatList->clear();
    for (int c = 1; c <= t.carriages(); ++c) {
        const QVector<int> seats = t.availableSeats(c);
        for (int s : seats)
            ui->seatList->addItem(QString("%1号车厢 %2号座").arg(c).arg(s));
    }

    ui->stopList->clear();
    for (const QString &s : t.stops())
        ui->stopList->addItem(s);
}