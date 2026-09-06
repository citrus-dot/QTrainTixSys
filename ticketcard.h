#ifndef TICKETCARD_H
#define TICKETCARD_H

#include <QWidget>
#include <QString>
#include "train.h"

// 仿真中国铁路磁介质车票（浅蓝偏紫底纹 + 动车组水印 + 右侧齿孔副联）。
// 布局参照真实车票票面：红字票号/检票口 → 站名+拼音 → 日期座位 →
// 票价席位 → 限乘当日当次车 → 证件姓名 → 服务标语 → 售票码/售票站；
// 右侧副联（齿孔虚线分隔）为二维码 + 订单号 + 支付信息。
// 全部 QPainter 绘制；订单号、支付信息在构造时生成，供保存车票时一并输出。
class TicketCard : public QWidget
{
    Q_OBJECT
public:
    explicit TicketCard(const Train *train, const QString &name, const QString &id,
                        int carriage, int seatNo, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override { return {780, 380}; }
    QSize sizeHint() const override { return {880, 430}; }

    // 订单 / 支付信息（保存车票时写入文件）
    QString orderNo() const { return m_orderNo; }
    QString ticketNo() const { return m_ticketNo; }   // 红字票号
    QString saleCode() const { return m_saleCode; }   // 21 位售票码
    QString payMethod() const { return m_payMethod; }
    QString txnNo() const { return m_txnNo; }         // 支付流水号
    QString payTime() const { return m_payTime; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawQr(QPainter &p, const QRectF &area) const;

    const Train *m_train;
    QString m_name;
    QString m_id;
    int m_carriage;
    int m_seatNo;

    QString m_orderNo;
    QString m_ticketNo;
    QString m_saleCode;
    QString m_payMethod;
    QString m_txnNo;
    QString m_payTime;
};

#endif // TICKETCARD_H
