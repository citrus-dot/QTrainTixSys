#include "addtraindialog.h"
#include "ui_addtraindialog.h"
#include <QMessageBox>
#include <QDate>

AddTrainDialog::AddTrainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddTrainDialog)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());
    connect(ui->carriagesSpin, &QSpinBox::valueChanged, this, [this](int value) {
        ui->firstClassSpin->setMaximum(value);
    });
}

AddTrainDialog::~AddTrainDialog()
{
    delete ui;
}

Train AddTrainDialog::getTrain() const
{
    Train t(ui->noEdit->text().trimmed(),
            ui->dateEdit->date().toString("yyyy-MM-dd"),
            ui->timeEdit->text().trimmed(),
            ui->fromEdit->text().trimmed(),
            ui->toEdit->text().trimmed(),
            ui->carriagesSpin->value(),
            ui->seatsSpin->value(),
            ui->firstPriceSpin->value(),
            ui->secondPriceSpin->value());
    QVector<int> classes;
    int firstCount = ui->firstClassSpin->value();
    for (int i = 0; i < t.carriages(); ++i)
        classes << (i < firstCount ? 1 : 2);
    t.setCarriageClass(classes);
    QStringList stops;
    for (const QString &s : ui->stopsEdit->text().split(',', Qt::SkipEmptyParts))
        stops << s.trimmed();
    t.setStops(stops);
    return t;
}

void AddTrainDialog::accept()
{
    if (ui->noEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "班次号不能为空");
        return;
    }
    if (ui->fromEdit->text().trimmed().isEmpty() || ui->toEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "发车城市和终点城市不能为空");
        return;
    }
    if (!Train::isValidTime(ui->timeEdit->text().trimmed())) {
        QMessageBox::warning(this, "提示", "发车时间格式应为 HH:MM");
        return;
    }
    if (ui->firstPriceSpin->value() <= 0 || ui->secondPriceSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "票价必须大于 0");
        return;
    }
    QDialog::accept();
}
