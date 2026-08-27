#include "ticketdialog.h"
#include "ui_ticketdialog.h"
#include <QMessageBox>

TicketDialog::TicketDialog(Train *train, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TicketDialog)
    , m_train(train)
{
    ui->setupUi(this);
    ui->carriageSpin->setMaximum(train->carriages());
    ui->seatSpin->setMaximum(train->seatsPerCarriage());

    // 退票时无需填写旅客信息
    connect(ui->sellRadio, &QRadioButton::toggled, this, [this](bool checked) {
        ui->nameEdit->setEnabled(checked);
        ui->idEdit->setEnabled(checked);
    });
}

TicketDialog::~TicketDialog()
{
    delete ui;
}

void TicketDialog::setSellMode(bool sell)
{
    ui->sellRadio->setChecked(sell);
    ui->refundRadio->setChecked(!sell);
}

bool TicketDialog::isSell() const { return ui->sellRadio->isChecked(); }
QString TicketDialog::name() const { return ui->nameEdit->text().trimmed(); }
QString TicketDialog::id() const { return ui->idEdit->text().trimmed(); }
int TicketDialog::carriage() const { return ui->carriageSpin->value(); }
int TicketDialog::seatNo() const { return ui->seatSpin->value(); }

void TicketDialog::accept()
{
    int carriage = ui->carriageSpin->value();
    int seatNo = ui->seatSpin->value();
    int index = (carriage - 1) * m_train->seatsPerCarriage() + (seatNo - 1);
    bool occupied = !m_train->seats()[index].isEmpty();

    if (ui->sellRadio->isChecked()) {
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
