#ifndef STATISTICSPAGE_H
#define STATISTICSPAGE_H

#include <QWidget>
#include <QVector>
#include "train.h"

class DonutChart;
class QLabel;
class QProgressBar;
class QTableWidget;

// 统计页面：环形图 + 等级分布图 + 班次列表表格
class StatisticsPage : public QWidget
{
    Q_OBJECT
public:
    explicit StatisticsPage(QWidget *parent = nullptr);

    void setData(const QVector<Train> &trains);

private:
    struct ClassStats {
        int firstCarriages;
        int firstSeats;
        int secondCarriages;
        int secondSeats;
    };

    ClassStats calcClassStats(const QVector<Train> &trains);

    DonutChart *m_donutChart;
    QLabel *m_trainCount;
    QLabel *m_capacityLabel;
    QLabel *m_soldLabel;
    QLabel *m_remainingLabel;
    QProgressBar *m_firstProgress;
    QProgressBar *m_secondProgress;
    QLabel *m_firstLabel;
    QLabel *m_secondLabel;
    QTableWidget *m_trainTable;
};

#endif // STATISTICSPAGE_H