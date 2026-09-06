#ifndef TRAINSYSTEM_H
#define TRAINSYSTEM_H

#include <QString>
#include <QVector>
#include "train.h"

// 文档类：管理所有班次
class TrainSystem
{
public:
    bool addTrain(const Train &t);       // 班次号不重复
    // 修改班次：用 newTrain 替换 oldNo。新班次号查重（排除自身）；
    // 已售座位必须仍处于新编组范围内，否则拒绝修改；成功时保留原售座记录
    bool updateTrain(const QString &oldNo, const Train &newTrain);
    bool removeTrain(const QString &no);
    Train *findTrain(const QString &no);
    const QVector<Train> &trains() const;

    bool saveToFile(const QString &path) const;
    bool loadFromFile(const QString &path);

private:
    int indexOf(const QString &no) const;

    QVector<Train> m_trains;
};

#endif // TRAINSYSTEM_H
