#ifndef TABLEUTIL_H
#define TABLEUTIL_H

#include <QLayout>
#include <QGridLayout>
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

// ---------- 座位网格过道（TicketDialog / SeatMapPopup / SeatTableView 共用）----------

// 座位排 r 在 QGridLayout 中的实际行号：过道空行插在 aisleAfter 排之后，
// 其后的座位排整体下移一行（两排→过道居中；三排→过道在第二、三排之间）
inline int seatGridRow(int seatRow, int aisleAfter)
{
    return seatRow + (seatRow > aisleAfter ? 1 : 0);
}

// 过道分隔条：上下两条浅灰细线示意过道边界（样式见 theme.qss #aisleFrame）
inline QFrame *makeAisleFrame(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName("aisleFrame");
    frame->setFixedHeight(12);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    return frame;
}

// 在座位网格中插入过道空行（rows 为座位排数，rows < 2 时不画）
inline void addAisleRow(QGridLayout *grid, int rows, int cols, QWidget *parent)
{
    if (rows >= 2)
        grid->addWidget(makeAisleFrame(parent), rows - 1, 0, 1, cols);
}

#endif // TABLEUTIL_H
