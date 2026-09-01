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
    Train(const QString &no, const QString &date, const QString &departTime, const QString &from,
          const QString &to, int carriages, int seatsPerCarriage,
          double firstClassPrice = 0.0, double secondClassPrice = 0.0);

    bool sellTicket(const QString &name, const QString &id, int carriage, int seatNo); // 售票，座位号不重复
    bool refundTicket(int carriage, int seatNo);                                       // 退票

    int remainingSeats() const;          // 余票数
    QVector<int> availableSeats(int carriage) const; // 指定车厢的可选座位号

    QString no() const;
    QString date() const;
    QString departTime() const;
    QString from() const;
    QString to() const;
    int carriages() const;
    int seatsPerCarriage() const;
    const QVector<Seat> &seats() const;

    void setStops(const QStringList &stops); // 中间停靠（可选提升）
    QStringList stops() const;

    double firstClassPrice() const;  // 一等票价
    double secondClassPrice() const; // 二等票价
    int carriageClass(int carriage) const;          // 1=一等 2=二等
    void setCarriageClass(const QVector<int> &classes);
    QString carriageClassText(int carriage) const;  // "一等座"/"二等座"
    double priceOf(int carriage) const;             // 按车厢等级返回票价
    bool isSeatOccupied(int carriage, int seatNo) const;

    static bool isValidTime(const QString &t);  // HH:MM
    static bool isValidDate(const QString &d);  // YYYY-MM-DD
    static bool isValidId(const QString &id);   // 18 位身份证号

private:
    QString m_no;
    QString m_date;
    QString m_departTime;
    QString m_from;
    QString m_to;
    int m_carriages = 0;
    int m_seatsPerCarriage = 0;
    double m_firstClassPrice = 0.0;
    double m_secondClassPrice = 0.0;
    QVector<int> m_carriageClass; // 每车厢等级，1=一等 2=二等
    QVector<Seat> m_seats;
    QStringList m_stops;
};

#endif // TRAIN_H
