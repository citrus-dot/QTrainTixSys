#ifndef TRAIN_H
#define TRAIN_H

#include <QString>
#include <QVector>
#include <QStringList>
#include "seat.h"

// 班次类：班次基本信息 + 座位集合
class Train
{
public:
    Train() = default;
    Train(const QString &no, const QString &departTime, const QString &from,
          const QString &to, int carriages, int seatsPerCarriage);

    bool sellTicket(const QString &name, const QString &id, int carriage, int seatNo); // 售票，座位号不重复
    bool refundTicket(int carriage, int seatNo);                                       // 退票

    int remainingSeats() const;          // 余票数
    QVector<int> availableSeats() const; // 可选座位号列表

    QString no() const;
    QString departTime() const;
    QString from() const;
    QString to() const;
    int carriages() const;
    int seatsPerCarriage() const;
    const QVector<Seat> &seats() const;

    void setStops(const QStringList &stops); // 中间停靠（可选提升）
    QStringList stops() const;

private:
    QString m_no;
    QString m_departTime;
    QString m_from;
    QString m_to;
    int m_carriages = 0;
    int m_seatsPerCarriage = 0;
    QVector<Seat> m_seats;
    QStringList m_stops;
};

#endif // TRAIN_H
