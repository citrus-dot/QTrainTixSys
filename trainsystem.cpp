#include "trainsystem.h"
#include <QFile>
#include <QTextStream>

bool TrainSystem::addTrain(const Train &t)
{
    if (findTrain(t.no()))
        return false; // 班次号不能重复
    m_trains.append(t);
    return true;
}

bool TrainSystem::removeTrain(const QString &no)
{
    for (int i = 0; i < m_trains.size(); ++i) {
        if (m_trains[i].no() == no) {
            m_trains.removeAt(i);
            return true;
        }
    }
    return false;
}

Train *TrainSystem::findTrain(const QString &no)
{
    for (Train &t : m_trains)
        if (t.no() == no)
            return &t;
    return nullptr;
}

const QVector<Train> &TrainSystem::trains() const { return m_trains; }

// 文件格式：班次数 / 班次字段(班次号 发车时间 发车城市 终点城市 车厢数 每厢座位数 日期 一等票价 二等票价 各车厢等级... 停靠站数 停靠站...) / 已售座位数 / 座位字段(姓名 身份证号 车厢号 座位号)
bool TrainSystem::saveToFile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << m_trains.size() << '\n';
    for (const Train &t : m_trains) {
        out << t.no() << ' ' << t.departTime() << ' ' << t.from() << ' ' << t.to()
            << ' ' << t.carriages() << ' ' << t.seatsPerCarriage()
            << ' ' << t.date() << ' ' << t.firstClassPrice() << ' ' << t.secondClassPrice();
        for (int c = 1; c <= t.carriages(); ++c)
            out << ' ' << t.carriageClass(c);
        const QStringList stops = t.stops();
        out << ' ' << stops.size();
        for (const QString &s : stops)
            out << ' ' << s;
        out << '\n';
        int sold = 0;
        for (const Seat &s : t.seats())
            if (!s.isEmpty())
                ++sold;
        out << sold << '\n';
        for (const Seat &s : t.seats())
            if (!s.isEmpty())
                out << s.name() << ' ' << s.id() << ' ' << s.carriage() << ' ' << s.seatNo() << '\n';
    }
    return true;
}

bool TrainSystem::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    int trainCount;
    in >> trainCount;
    m_trains.clear();
    for (int i = 0; i < trainCount; ++i) {
        QString no, departTime, from, to;
        int carriages, seatsPerCarriage;
        in >> no >> departTime >> from >> to >> carriages >> seatsPerCarriage;

        // 向后兼容：下一字段是日期(新格式)或停靠站数(旧格式)
        QString next;
        in >> next;
        QString date;
        double firstPrice = 0.0, secondPrice = 0.0;
        QVector<int> classes;
        int stopCount;
        if (Train::isValidDate(next)) {
            date = next;
            in >> firstPrice >> secondPrice;
            for (int j = 0; j < carriages; ++j) {
                int cls;
                in >> cls;
                classes << cls;
            }
            in >> stopCount;
        } else {
            stopCount = next.toInt();
        }

        Train t(no, date, departTime, from, to, carriages, seatsPerCarriage, firstPrice, secondPrice);
        if (!classes.isEmpty())
            t.setCarriageClass(classes);
        QStringList stops;
        for (int j = 0; j < stopCount; ++j) {
            QString s;
            in >> s;
            stops << s;
        }
        t.setStops(stops);
        int sold;
        in >> sold;
        for (int j = 0; j < sold; ++j) {
            QString name, id;
            int carriage, seatNo;
            in >> name >> id >> carriage >> seatNo;
            t.sellTicket(name, id, carriage, seatNo);
        }
        m_trains.append(t);
    }
    return true;
}
