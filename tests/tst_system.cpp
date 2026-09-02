#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "trainsystem.h"

class TestTrainSystem : public QObject
{
    Q_OBJECT
private slots:
    void addTrainSuccess();
    void addTrainDuplicate();
    void removeTrain();
    void findTrain();
    void fileRoundTrip();
    void oldFormatCompatibility();
};

void TestTrainSystem::addTrainSuccess()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(sys.addTrain(t));
    QCOMPARE(sys.trains().size(), 1);
}

void TestTrainSystem::addTrainDuplicate()
{
    TrainSystem sys;
    Train t1("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    Train t2("G100", "2026-09-02", "09:00", "北京", "上海", 1, 2);
    QVERIFY(sys.addTrain(t1));
    QVERIFY(!sys.addTrain(t2));
    QCOMPARE(sys.trains().size(), 1);
}

void TestTrainSystem::removeTrain()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    sys.addTrain(t);
    QVERIFY(sys.removeTrain("G100"));
    QCOMPARE(sys.trains().size(), 0);
    QVERIFY(!sys.removeTrain("G100"));
}

void TestTrainSystem::findTrain()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    sys.addTrain(t);
    QVERIFY(sys.findTrain("G100") != nullptr);
    QCOMPARE(sys.findTrain("G100")->from(), QString("南京"));
    QVERIFY(sys.findTrain("NOPE") == nullptr);
}

void TestTrainSystem::fileRoundTrip()
{
    TrainSystem sys;
    Train g("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3, 100.0, 60.0);
    g.setCarriageClass({1, 2});
    g.setStops({"无锡", "苏州"});
    g.sellTicket("张三", "110101199001011234", 1, 1);
    g.sellTicket("李四", "110101199001011235", 2, 2);
    QVERIFY(sys.addTrain(g));
    Train k("K200", "2026-09-02", "10:30", "北京", "广州", 1, 2);
    QVERIFY(sys.addTrain(k));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("data.dat");
    QVERIFY(sys.saveToFile(path));

    TrainSystem loaded;
    QVERIFY(loaded.loadFromFile(path));
    QCOMPARE(loaded.trains().size(), 2);

    Train *lg = loaded.findTrain("G100");
    QVERIFY(lg);
    QCOMPARE(lg->date(), QString("2026-09-01"));
    QCOMPARE(lg->departTime(), QString("08:00"));
    QCOMPARE(lg->from(), QString("南京"));
    QCOMPARE(lg->to(), QString("上海"));
    QCOMPARE(lg->carriages(), 2);
    QCOMPARE(lg->seatsPerCarriage(), 3);
    QCOMPARE(lg->firstClassPrice(), 100.0);
    QCOMPARE(lg->secondClassPrice(), 60.0);
    QCOMPARE(lg->carriageClass(1), 1);
    QCOMPARE(lg->carriageClass(2), 2);
    QCOMPARE(lg->stops(), QStringList({"无锡", "苏州"}));
    QVERIFY(lg->isSeatOccupied(1, 1));
    QCOMPARE(lg->seats()[0].name(), QString("张三"));
    QVERIFY(lg->isSeatOccupied(2, 2));
    QCOMPARE(lg->seats()[4].name(), QString("李四"));
    QCOMPARE(lg->remainingSeats(), 4);

    Train *lk = loaded.findTrain("K200");
    QVERIFY(lk);
    QCOMPARE(lk->date(), QString("2026-09-02"));
    QCOMPARE(lk->carriages(), 1);
    QCOMPARE(lk->remainingSeats(), 2);
}

void TestTrainSystem::oldFormatCompatibility()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("old.dat");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "1\n";
        out << "K100 09:00 北京 上海 1 3 0\n";
        out << "0\n";
    }
    TrainSystem sys;
    QVERIFY(sys.loadFromFile(path));
    QCOMPARE(sys.trains().size(), 1);
    Train *t = sys.findTrain("K100");
    QVERIFY(t);
    QCOMPARE(t->date(), QString());
    QCOMPARE(t->departTime(), QString("09:00"));
    QCOMPARE(t->carriages(), 1);
    QCOMPARE(t->seatsPerCarriage(), 3);
    QCOMPARE(t->remainingSeats(), 3);
    QVERIFY(t->stops().isEmpty());
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestTrainSystem test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_system.moc"
