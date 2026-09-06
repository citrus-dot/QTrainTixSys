#ifndef TABLEUTIL_H
#define TABLEUTIL_H

#include <QLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QVector>
#include <QTableWidgetItem>
#include <QString>

// UI 共享辅助：居中单元格、按内容权重铺满列宽、清空布局、身份证脱敏展示。
// 由 TrainListView / SeatStatusTable / StatisticsPage / TicketCard 等共用。

// 身份证打码（仅用于界面展示与导出，数据文件仍存完整号）：
// 15 位以上号码第 11-14 位替换为 *（与真实车票一致），短号保留前 6 后 4
inline QString maskId(const QString &id)
{
    if (id.size() <= 10)
        return id;
    if (id.size() >= 15) {
        QString s = id;
        for (int i = 10; i <= 13 && i < s.size() - 4; ++i)
            s[i] = QLatin1Char('*');
        return s;
    }
    return id.left(6) + QString(id.size() - 10, QLatin1Char('*')) + id.right(4);
}

inline QTableWidgetItem *makeCenteredItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

// 测量各列内容宽度，作为列宽权重（数据刷新后调用一次）
inline void computeColumnWeights(QTableWidget *table, QVector<int> &weights)
{
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->resizeColumnsToContents();
    weights.clear();
    for (int i = 0; i < table->columnCount(); ++i)
        weights.append(qMax(1, table->columnWidth(i)));
}

// 按权重把各列铺满视口宽度（showEvent / resizeEvent 时调用）
inline void applyWeightedColumnWidths(QTableWidget *table, const QVector<int> &weights)
{
    if (weights.isEmpty() || weights.size() < table->columnCount() || table->rowCount() == 0)
        return;
    const int avail = table->viewport()->width();
    if (avail <= 0)
        return;
    int total = 0;
    for (int w : weights)
        total += w;
    int assigned = 0;
    for (int i = 0; i < table->columnCount(); ++i) {
        const int w = (i == table->columnCount() - 1)
            ? avail - assigned
            : avail * weights[i] / total;
        table->setColumnWidth(i, w);
        assigned += w;
    }
}

// 清空布局内所有子项（座位网格重建时使用）
inline void clearLayout(QLayout *layout)
{
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

#endif // TABLEUTIL_H
