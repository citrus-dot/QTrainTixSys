#include "addtraindialog.h"
#include "ui_addtraindialog.h"
#include <QMessageBox>

AddTrainDialog::AddTrainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddTrainDialog)
{
    ui->setupUi(this);
}

AddTrainDialog::~AddTrainDialog()
{
    delete ui;
}

Train AddTrainDialog::getTrain() const
{
    Train t(ui->noEdit->text().trimmed(),
            ui->timeEdit->text().trimmed(),
            ui->fromEdit->text().trimmed(),
            ui->toEdit->text().trimmed(),
            ui->carriagesSpin->value(),
            ui->seatsSpin->value());
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
    QDialog::accept();
}
