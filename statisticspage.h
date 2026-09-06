#ifndef STATISTICSPAGE_H
#define STATISTICSPAGE_H

#include <QWidget>
#include <QVector>
#include "train.h"

class DonutChart;
class QLabel;
class QProgressBar;
class QTableWidget;
class QShowEvent;
class QResizeEvent;

// 统计页面：环形图 + 等级分布图 + 班次列表表格
class StatisticsPage : public QWidget
{
    Q_OBJECT
public:
    explicit StatisticsPage(QWidget *parent = nullptr);

    void setData(const QVector<Train> &trains);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct ClassStats {
        int firstCarriages;
        int firstSeats;
        int secondCarriages;
        int secondSeats;
    };

    ClassStats calcClassStats(const QVector<Train> &trains);
    void animateBarTo(QProgressBar *bar, int target); // 占比条入场动画（0 → target）
    void applyColumnWidths();

    DonutChart *m_donutChart;
    QProgressBar *m_firstProgress;
    QProgressBar *m_secondProgress;
    QLabel *m_firstLabel;
    QLabel *m_secondLabel;
    QTableWidget *m_trainTable;
    QVector<int> m_weights;
};

#endif // STATISTICSPAGE_H