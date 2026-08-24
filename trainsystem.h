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
    bool removeTrain(const QString &no);
    Train *findTrain(const QString &no);
    const QVector<Train> &trains() const;

    bool saveToFile(const QString &path) const;
    bool loadFromFile(const QString &path);

private:
    QVector<Train> m_trains;
};

#endif // TRAINSYSTEM_H
