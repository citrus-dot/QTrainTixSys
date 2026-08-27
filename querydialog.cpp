#include "querydialog.h"
#include "ui_querydialog.h"

QueryDialog::QueryDialog(const TrainSystem &system, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QueryDialog)
    , m_system(system)
{
    ui->setupUi(this);
    for (const Train &t : m_system.trains())
        ui->comboTrain->addItem(t.no());
    connect(ui->comboTrain, &QComboBox::currentIndexChanged, this, &QueryDialog::onTrainChanged);
    onTrainChanged();
}

QueryDialog::~QueryDialog()
{
    delete ui;
}

void QueryDialog::onTrainChanged()
{
    int index = ui->comboTrain->currentIndex();
    if (index < 0 || index >= m_system.trains().size()) {
        ui->infoLabel->setText("暂无班次");
        ui->seatList->clear();
        ui->stopList->clear();
        return;
    }
    const Train &t = m_system.trains()[index];
    ui->infoLabel->setText(QString("日期: %1   发车时间: %2   发车城市: %3   终点城市: %4   余票数: %5")
                           .arg(t.date(), t.departTime(), t.from(), t.to())
                           .arg(t.remainingSeats()));

    QStringList firstClassList;
    for (int c = 1; c <= t.carriages(); ++c)
        if (t.carriageClass(c) == 1)
            firstClassList << QString::number(c);
    QString classInfo = firstClassList.isEmpty()
        ? "一等座: 无"
        : "一等座: " + firstClassList.join(",") + "号车厢";
    ui->priceLabel->setText(QString("一等票价: %1 元   二等票价: %2 元   %3")
                            .arg(t.firstClassPrice(), 0, 'f', 2)
                            .arg(t.secondClassPrice(), 0, 'f', 2)
                            .arg(classInfo));

    ui->seatList->clear();
    const QVector<int> seats = t.availableSeats();
    for (int s : seats)
        ui->seatList->addItem(QString::number(s));

    ui->stopList->clear();
    const QStringList stops = t.stops();
    for (const QString &s : stops)
        ui->stopList->addItem(s);
}
