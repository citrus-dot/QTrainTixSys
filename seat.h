#ifndef SEAT_H
#define SEAT_H

#include <QString>

// 座位类：一个座位 + 旅客信息
class Seat
{
public:
    Seat() = default;
    Seat(const QString &name, const QString &id, int carriage, int seatNo);

    bool isEmpty() const;              // 座位空闲判断
    QString name() const;
    QString id() const;
    int carriage() const;
    int seatNo() const;

    void setPassenger(const QString &name, const QString &id);
    void clearPassenger();

private:
    QString m_name;
    QString m_id;
    int m_carriage = 0;
    int m_seatNo = 0;
};

#endif // SEAT_H
