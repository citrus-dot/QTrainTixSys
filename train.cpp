#include "train.h"

Train::Train(const QString &no, const QString &departTime, const QString &from,
             const QString &to, int carriages, int seatsPerCarriage)
    : m_no(no), m_departTime(departTime), m_from(from), m_to(to),
      m_carriages(carriages), m_seatsPerCarriage(seatsPerCarriage)
{
    m_seats.resize(carriages * seatsPerCarriage);
    for (int i = 0; i < m_seats.size(); ++i)
        m_seats[i] = Seat(QString(), QString(), i / seatsPerCarriage + 1, i % seatsPerCarriage + 1);
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

QVector<int> Train::availableSeats() const
{
    QVector<int> result;
    for (int i = 0; i < m_seats.size(); ++i)
        if (m_seats[i].isEmpty())
            result.append(i + 1);
    return result;
}

QString Train::no() const { return m_no; }
QString Train::departTime() const { return m_departTime; }
QString Train::from() const { return m_from; }
QString Train::to() const { return m_to; }
int Train::carriages() const { return m_carriages; }
int Train::seatsPerCarriage() const { return m_seatsPerCarriage; }
const QVector<Seat> &Train::seats() const { return m_seats; }

void Train::setStops(const QStringList &stops) { m_stops = stops; }
QStringList Train::stops() const { return m_stops; }
