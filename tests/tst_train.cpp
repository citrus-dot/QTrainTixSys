#include <QtTest>
#include "seat.h"
#include "train.h"

class TestSeat : public QObject
{
    Q_OBJECT
private slots:
    void defaultSeatIsEmpty();
    void setAndClearPassenger();
    void constructorSetsFields();
};

class TestTrain : public QObject
{
    Q_OBJECT
private slots:
    void sellTicketSuccess();
    void sellTicketDuplicate();
    void sellTicketOutOfRange();
    void refundTicketSuccess();
    void refundTicketEmpty();
    void refundTicketOutOfRange();
    void remainingSeatsCount();
    void availableSeatsPerCarriage();
    void priceAndClass();
    void validation();
};

void TestSeat::defaultSeatIsEmpty()
{
    Seat s;
    QVERIFY(s.isEmpty());
    QCOMPARE(s.name(), QString());
    QCOMPARE(s.id(), QString());
}

void TestSeat::setAndClearPassenger()
{
    Seat s;
    s.setPassenger("张三", "110101199001011234");
    QVERIFY(!s.isEmpty());
    QCOMPARE(s.name(), QString("张三"));
    QCOMPARE(s.id(), QString("110101199001011234"));
    s.clearPassenger();
    QVERIFY(s.isEmpty());
}

void TestSeat::constructorSetsFields()
{
    Seat s("李四", "110101199001011235", 2, 5);
    QCOMPARE(s.carriage(), 2);
    QCOMPARE(s.seatNo(), 5);
    QCOMPARE(s.name(), QString("李四"));
    QVERIFY(!s.isEmpty());
}

void TestTrain::sellTicketSuccess()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(t.sellTicket("张三", "110101199001011234", 1, 1));
    QVERIFY(t.isSeatOccupied(1, 1));
    QCOMPARE(t.seats()[0].name(), QString("张三"));
}

void TestTrain::sellTicketDuplicate()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(t.sellTicket("张三", "110101199001011234", 1, 1));
    QVERIFY(!t.sellTicket("李四", "110101199001011235", 1, 1));
}

void TestTrain::sellTicketOutOfRange()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(!t.sellTicket("张三", "110101199001011234", 0, 1));
    QVERIFY(!t.sellTicket("张三", "110101199001011234", 3, 1));
    QVERIFY(!t.sellTicket("张三", "110101199001011234", 1, 0));
    QVERIFY(!t.sellTicket("张三", "110101199001011234", 1, 4));
}

void TestTrain::refundTicketSuccess()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    t.sellTicket("张三", "110101199001011234", 1, 1);
    QVERIFY(t.refundTicket(1, 1));
    QVERIFY(!t.isSeatOccupied(1, 1));
}

void TestTrain::refundTicketEmpty()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(!t.refundTicket(1, 1));
}

void TestTrain::refundTicketOutOfRange()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QVERIFY(!t.refundTicket(0, 1));
    QVERIFY(!t.refundTicket(1, 4));
}

void TestTrain::remainingSeatsCount()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    QCOMPARE(t.remainingSeats(), 6);
    t.sellTicket("张三", "110101199001011234", 1, 1);
    t.sellTicket("李四", "110101199001011235", 2, 2);
    QCOMPARE(t.remainingSeats(), 4);
    t.refundTicket(1, 1);
    QCOMPARE(t.remainingSeats(), 5);
}

void TestTrain::availableSeatsPerCarriage()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 2, 3);
    t.sellTicket("张三", "110101199001011234", 1, 2);
    QCOMPARE(t.availableSeats(1), QVector<int>({1, 3}));
    QCOMPARE(t.availableSeats(2), QVector<int>({1, 2, 3}));
    QVERIFY(t.availableSeats(0).isEmpty());
    QVERIFY(t.availableSeats(3).isEmpty());
}

void TestTrain::priceAndClass()
{
    Train t("G100", "2026-09-01", "08:00", "南京", "上海", 3, 3, 100.0, 60.0);
    QCOMPARE(t.carriageClass(1), 2);
    QCOMPARE(t.priceOf(1), 60.0);
    QCOMPARE(t.carriageClassText(1), QString("二等座"));
    t.setCarriageClass({1, 2, 1});
    QCOMPARE(t.carriageClass(1), 1);
    QCOMPARE(t.carriageClass(3), 1);
    QCOMPARE(t.priceOf(1), 100.0);
    QCOMPARE(t.priceOf(2), 60.0);
    QCOMPARE(t.carriageClassText(1), QString("一等座"));
    QCOMPARE(t.carriageClassText(2), QString("二等座"));
}

void TestTrain::validation()
{
    QVERIFY(Train::isValidTime("08:00"));
    QVERIFY(!Train::isValidTime("8:00"));
    QVERIFY(!Train::isValidTime("24:00"));
    QVERIFY(!Train::isValidTime("08:60"));
    QVERIFY(Train::isValidDate("2026-09-01"));
    QVERIFY(!Train::isValidDate("2026-13-01"));
    QVERIFY(!Train::isValidDate("2026-09-32"));
    QVERIFY(!Train::isValidDate("2026/09/01"));
    QVERIFY(Train::isValidId("110101199001011234"));
    QVERIFY(Train::isValidId("11010119900101123X"));
    QVERIFY(!Train::isValidId("12345"));
    QVERIFY(!Train::isValidId("11010119900101123"));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;
    TestSeat seatTest;
    TestTrain trainTest;
    status |= QTest::qExec(&seatTest, argc, argv);
    status |= QTest::qExec(&trainTest, argc, argv);
    return status;
}

#include "tst_train.moc"
