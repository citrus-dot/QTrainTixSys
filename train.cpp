#include "train.h"
#include <QRegularExpression>
#include <QDate>

Train::Train(const QString &no, const QString &date, const QString &departTime, const QString &from,
             const QString &to, int carriages, int seatsPerCarriage,
             double firstClassPrice, double secondClassPrice)
    : m_no(no), m_date(date), m_departTime(departTime), m_from(from), m_to(to),
      m_carriages(carriages), m_seatsPerCarriage(seatsPerCarriage),
      m_firstClassPrice(firstClassPrice), m_secondClassPrice(secondClassPrice)
{
    m_seats.resize(carriages * seatsPerCarriage);
    for (int i = 0; i < m_seats.size(); ++i)
        m_seats[i] = Seat(QString(), QString(), i / seatsPerCarriage + 1, i % seatsPerCarriage + 1);
    m_carriageClass.fill(2, carriages);
}

bool Train::sellTicket(const QString &name, const QString &id, int carriage, int seatNo)
{
    if (carriage < 1 || carriage > m_carriages || seatNo < 1 || seatNo > m_seatsPerCarriage)
        return false;
    int index = (carriage - 1) * m_seatsPerCarriage + (seatNo - 1);
    if (!m_seats[index].isEmpty())
        return false; // 座位号不能重复
    m_seats[index].setPassenger(name, id);
    return true;
}

bool Train::refundTicket(int carriage, int seatNo)
{
    if (carriage < 1 || carriage > m_carriages || seatNo < 1 || seatNo > m_seatsPerCarriage)
        return false;
    int index = (carriage - 1) * m_seatsPerCarriage + (seatNo - 1);
    if (m_seats[index].isEmpty())
        return false;
    m_seats[index].clearPassenger();
    return true;
}

int Train::remainingSeats() const
{
    int count = 0;
    for (const Seat &s : m_seats)
        if (s.isEmpty())
            ++count;
    return count;
}

QVector<int> Train::availableSeats(int carriage) const
{
    QVector<int> result;
    if (carriage < 1 || carriage > m_carriages)
        return result;
    for (int s = 1; s <= m_seatsPerCarriage; ++s)
        if (!isSeatOccupied(carriage, s))
            result.append(s);
    return result;
}

QString Train::no() const { return m_no; }
QString Train::date() const { return m_date; }
QString Train::departTime() const { return m_departTime; }
QString Train::from() const { return m_from; }
QString Train::to() const { return m_to; }
int Train::carriages() const { return m_carriages; }
int Train::seatsPerCarriage() const { return m_seatsPerCarriage; }
const QVector<Seat> &Train::seats() const { return m_seats; }

void Train::setStops(const QStringList &stops) { m_stops = stops; }
QStringList Train::stops() const { return m_stops; }

double Train::firstClassPrice() const { return m_firstClassPrice; }
double Train::secondClassPrice() const { return m_secondClassPrice; }

int Train::carriageClass(int carriage) const
{
    if (carriage < 1 || carriage > m_carriageClass.size())
        return 2;
    return m_carriageClass[carriage - 1];
}

void Train::setCarriageClass(const QVector<int> &classes)
{
    m_carriageClass = classes;
    if (m_carriageClass.size() < m_carriages)
        m_carriageClass.resize(m_carriages, 2);
}

QString Train::carriageClassText(int carriage) const
{
    return carriageClass(carriage) == 1 ? "一等座" : "二等座";
}

double Train::priceOf(int carriage) const
{
    return carriageClass(carriage) == 1 ? m_firstClassPrice : m_secondClassPrice;
}

bool Train::isSeatOccupied(int carriage, int seatNo) const
{
    if (carriage < 1 || carriage > m_carriages || seatNo < 1 || seatNo > m_seatsPerCarriage)
        return false;
    int index = (carriage - 1) * m_seatsPerCarriage + (seatNo - 1);
    return !m_seats[index].isEmpty();
}

const Seat &Train::seatAt(int carriage, int seatNo) const
{
    static const Seat empty;
    if (carriage < 1 || carriage > m_carriages || seatNo < 1 || seatNo > m_seatsPerCarriage)
        return empty;
    return m_seats[(carriage - 1) * m_seatsPerCarriage + (seatNo - 1)];
}

bool Train::isValidTime(const QString &t)
{
    return QRegularExpression("^([01]\\d|2[0-3]):[0-5]\\d$").match(t).hasMatch();
}

bool Train::isValidDate(const QString &d)
{
    return QDate::fromString(d, "yyyy-MM-dd").isValid();
}

bool Train::isValidId(const QString &id)
{
    // 18 位：17 位数字 + 数字或 X/x
    return QRegularExpression("^\\d{17}[0-9Xx]$").match(id).hasMatch();
}
