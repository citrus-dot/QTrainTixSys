#include "seat.h"

Seat::Seat(const QString &name, const QString &id, int carriage, int seatNo)
    : m_name(name), m_id(id), m_carriage(carriage), m_seatNo(seatNo) {}

bool Seat::isEmpty() const { return m_name.isEmpty(); }

QString Seat::name() const { return m_name; }
QString Seat::id() const { return m_id; }
int Seat::carriage() const { return m_carriage; }
int Seat::seatNo() const { return m_seatNo; }

void Seat::setPassenger(const QString &name, const QString &id)
{
    m_name = name;
    m_id = id;
}

void Seat::clearPassenger()
{
    m_name.clear();
    m_id.clear();
}
