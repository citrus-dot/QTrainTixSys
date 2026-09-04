#ifndef SEATSTATUSTABLE_H
#define SEATSTATUSTABLE_H

#include <QWidget>
#include <QVector>
#include "train.h"

class QTableWidget;

// 座位状态表格：以表格列出票务状态，左侧色块标状态，仅可操作座位可选中
class SeatStatusTable : public QWidget
{
    Q_OBJECT
public:
    explicit SeatStatusTable(QWidget *parent = nullptr);

    void setTrain(Train *train);
    void setSellMode(bool sell); // true=售票(空座可操作) false=退票(已售可操作)

    int selectedCarriage() const;
    int selectedSeat() const;

signals:
    void selectionChanged();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void rebuild();
    void applyColumnWidths();

    Train *m_train = nullptr;
    bool m_sell = true;
    QTableWidget *m_table;
    QVector<int> m_weights; // 各列内容权重（rebuild 时计算）
    int m_totalWeight = 0;
};

#endif // SEATSTATUSTABLE_H
