#include "ticketview.h"
#include "ui_ticketview.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QStyle>

TicketView::TicketView(const Train *train, const QString &name, const QString &id,
                       int carriage, int seatNo, QWidget *parent)
    : FadeDialog(parent)
    , ui(new Ui::TicketView)
    , m_train(train)
    , m_name(name)
    , m_id(id)
    , m_carriage(carriage)
    , m_seatNo(seatNo)
{
    ui->setupUi(this);
    ui->noLabel->setText(train->no());
    ui->dateLabel->setText(train->date());
    ui->timeLabel->setText(train->departTime());
    ui->routeLabel->setText(QString("%1 → %2").arg(train->from(), train->to()));
    ui->passengerLabel->setText(name);
    ui->idLabel->setText(id);
    ui->seatLabel->setText(QString("%1号车厢 %2号座").arg(carriage).arg(seatNo));
    ui->classLabel->setText(train->carriageClassText(carriage));
    ui->priceLabel->setText(QString("¥ %1").arg(train->priceOf(carriage), 0, 'f', 2));
    ui->saveButton->setProperty("primary", true);
    // 强制刷新样式，确保 QSS 属性选择器立即生效
    ui->saveButton->style()->unpolish(ui->saveButton);
    ui->saveButton->style()->polish(ui->saveButton);
    ui->saveButton->update();
    connect(ui->saveButton, &QPushButton::clicked, this, &TicketView::onSaveTicket);
}

TicketView::~TicketView()
{
    delete ui;
}

void TicketView::onSaveTicket()
{
    QString path = QFileDialog::getSaveFileName(this, "保存车票",
        QString("%1_%2_%3_%4.txt").arg(m_train->no(), m_name).arg(m_carriage).arg(m_seatNo),
        "文本文件 (*.txt)");
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "车票保存失败");
        return;
    }
    QTextStream out(&file);
    out << "==================== 列车车票 ====================\n";
    out << "班次号:   " << m_train->no() << "\n";
    out << "日期:     " << m_train->date() << "\n";
    out << "发车时间: " << m_train->departTime() << "\n";
    out << "行程:     " << m_train->from() << " → " << m_train->to() << "\n";
    out << "------------------------------------------------\n";
    out << "乘客:     " << m_name << "\n";
    out << "身份证号: " << m_id << "\n";
    out << "座位:     " << m_carriage << "号车厢 " << m_seatNo << "号座\n";
    out << "等级:     " << m_train->carriageClassText(m_carriage) << "\n";
    out << "票价:     " << QString::number(m_train->priceOf(m_carriage), 'f', 2) << " 元\n";
    out << "==================================================\n";
    QMessageBox::information(this, "提示", QString("车票已保存到:\n%1").arg(path));
}
