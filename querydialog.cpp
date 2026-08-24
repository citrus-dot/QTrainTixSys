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
    ui->infoLabel->setText(QString("发车时间: %1   发车城市: %2   终点城市: %3   余票数: %4")
                           .arg(t.departTime(), t.from(), t.to())
                           .arg(t.remainingSeats()));

    ui->seatList->clear();
    const QVector<int> seats = t.availableSeats();
    for (int s : seats)
        ui->seatList->addItem(QString::number(s));

    ui->stopList->clear();
    const QStringList stops = t.stops();
    for (const QString &s : stops)
        ui->stopList->addItem(s);
}
