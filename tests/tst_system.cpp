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
    void updateTrainRename();
    void updateTrainConflict();
    void updateTrainShrinkRejected();
    void updateTrainKeepsSeats();
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

void TestTrainSystem::updateTrainRename()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    sys.addTrain(t);
    Train renamed("G200", "2026-09-01", "08:30", "南京", "上海", 2, 3);
    QVERIFY(sys.updateTrain("G100", renamed));
    QVERIFY(sys.findTrain("G100") == nullptr);
    QVERIFY(sys.findTrain("G200") != nullptr);
}

void TestTrainSystem::updateTrainConflict()
{
    TrainSystem sys;
    sys.addTrain(Train("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3));
    sys.addTrain(Train("G200", "2026-09-02", "09:00", "北京", "广州", 2, 3));
    // 把 G100 改成已存在的 G200 → 拒绝
    Train conflict("G200", "2026-09-03", "10:00", "上海", "北京", 2, 3);
    QVERIFY(!sys.updateTrain("G100", conflict));
    QCOMPARE(sys.findTrain("G200")->departTime(), QString("09:00")); // 原班次未被覆盖
}

void TestTrainSystem::updateTrainShrinkRejected()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 5);
    sys.addTrain(t);
    sys.findTrain("G100")->sellTicket("张三", "110101199001011234", 2, 4); // 2号车厢4号座
    // 缩编到 1 节车厢 → 已售座位超出范围，必须拒绝
    Train shrink("G100", "2026-09-01", "08:00", "南京", "上海", 1, 5);
    QVERIFY(!sys.updateTrain("G100", shrink));
    // 缩小每厢座位数同理
    Train shrinkSeats("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(!sys.updateTrain("G100", shrinkSeats));
    // 范围刚好覆盖 → 允许
    Train ok("G100", "2026-09-01", "08:00", "南京", "上海", 2, 4);
    QVERIFY(sys.updateTrain("G100", ok));
    QVERIFY(sys.findTrain("G100")->isSeatOccupied(2, 4));
}

void TestTrainSystem::updateTrainKeepsSeats()
{
    TrainSystem sys;
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 5, 100.0, 60.0);
    sys.addTrain(t);
    sys.findTrain("G100")->sellTicket("张三", "110101199001011234", 1, 2);
    // 改时间不改编组：售座记录应完整保留
    Train edited("G100", "2026-09-01", "07:30", "南京", "上海", 2, 5, 100.0, 60.0);
    QVERIFY(sys.updateTrain("G100", edited));
    Train *p = sys.findTrain("G100");
    QCOMPARE(p->departTime(), QString("07:30"));
    QVERIFY(p->isSeatOccupied(1, 2));
    QCOMPARE(p->seats()[1].name(), QString("张三"));
    QCOMPARE(p->remainingSeats(), 9);
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

    // 旧格式班次再保存、再加载，应保持旧格式往返无损（无日期字段）
    const QString path2 = dir.filePath("old_roundtrip.dat");
    QVERIFY(sys.saveToFile(path2));
    TrainSystem sys2;
    QVERIFY(sys2.loadFromFile(path2));
    QCOMPARE(sys2.trains().size(), 1);
    Train *t2 = sys2.findTrain("K100");
    QVERIFY(t2);
    QCOMPARE(t2->date(), QString());
    QCOMPARE(t2->departTime(), QString("09:00"));
    QCOMPARE(t2->carriages(), 1);
    QCOMPARE(t2->seatsPerCarriage(), 3);
    QCOMPARE(t2->remainingSeats(), 3);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestTrainSystem test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_system.moc"
