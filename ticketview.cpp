#include "ticketview.h"
#include "ui_ticketview.h"
#include "ticketcard.h"
#include "tableutil.h"
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
    // 车票为自绘仿真票面（TicketCard），由构造函数插入 verticalLayout 首位
    m_card = new TicketCard(train, name, id, carriage, seatNo, this);
    ui->verticalLayout->insertWidget(0, m_card, 1);
    // 弹窗尺寸包住车票（.ui 里的 geometry 是旧表单布局的遗留值，比票面小）
    adjustSize();
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
    // 票面展示与保存文件共用同一份订单/支付数据（来自 TicketCard 构造时生成）
    QTextStream out(&file);
    out << "==================== 中国铁路 电子客票 ====================\n";
    out << "订单号:   " << m_card->orderNo() << "\n";
    out << "车票票号: " << m_card->ticketNo() << "\n";
    out << "售票码:   " << m_card->saleCode() << "\n";
    out << "----------------------------------------------------------\n";
    out << "班次号:   " << m_train->no() << "\n";
    out << "乘车日期: " << m_train->date() << "  " << m_train->departTime() << "开\n";
    out << "行程:     " << m_train->from() << "站 → " << m_train->to() << "站\n";
    out << "座位:     " << m_carriage << "号车厢 " << m_seatNo << "号\n";
    out << "席别:     " << m_train->carriageClassText(m_carriage) << "\n";
    out << "票价:     " << QString::number(m_train->priceOf(m_carriage), 'f', 1) << " 元\n";
    out << "----------------------------------------------------------\n";
    out << "乘客:     " << m_name << "\n";
    out << "身份证号: " << maskId(m_id) << "\n";
    out << "提示:     限乘当日当次车\n";
    out << "----------------------------------------------------------\n";
    out << "支付方式: " << m_card->payMethod() << "\n";
    out << "支付流水号: " << m_card->txnNo() << "\n";
    out << "支付时间: " << m_card->payTime() << "\n";
    out << "支付金额: ¥" << QString::number(m_train->priceOf(m_carriage), 'f', 2) << "\n";
    out << "==========================================================\n";
    QMessageBox::information(this, "提示", QString("车票已保存到:\n%1").arg(path));
}
