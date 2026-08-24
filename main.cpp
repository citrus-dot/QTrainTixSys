#include <QCoreApplication>
#include <QTextStream>
#include "trainsystem.h"

// V1 控制台自测：验证核心类逻辑与文件读写
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream out(stdout);

    TrainSystem system;

    // 1. 创建班次
    Train t1("T001", "08:00", "北京", "上海", 2, 5);
    t1.setStops({"济南", "南京"});
    system.addTrain(t1);
    system.addTrain(Train("T002", "09:30", "广州", "深圳", 1, 4));

    out << "班次数: " << system.trains().size() << '\n';

    // 2. 售票
    Train *t = system.findTrain("T001");
    out << "售票 1车1座: " << (t->sellTicket("张三", "110101199001011234", 1, 1) ? "成功" : "失败") << '\n';
    out << "售票 1车1座(重复): " << (t->sellTicket("李四", "110101199002021234", 1, 1) ? "成功" : "失败") << '\n';
    out << "售票 1车2座: " << (t->sellTicket("王五", "110101199003031234", 1, 2) ? "成功" : "失败") << '\n';

    // 3. 余票
    out << "T001 余票: " << t->remainingSeats() << '\n';

    // 4. 退票
    out << "退票 1车1座: " << (t->refundTicket(1, 1) ? "成功" : "失败") << '\n';
    out << "退票 1车1座(空座): " << (t->refundTicket(1, 1) ? "成功" : "失败") << '\n';
    out << "退票后余票: " << t->remainingSeats() << '\n';

    // 5. 文件读写
    system.saveToFile("trains.dat");
    TrainSystem system2;
    system2.loadFromFile("trains.dat");
    out << "读盘后班次数: " << system2.trains().size() << '\n';
    Train *t2 = system2.findTrain("T001");
    out << "读盘后 T001 余票: " << t2->remainingSeats() << '\n';

    return 0;
}
