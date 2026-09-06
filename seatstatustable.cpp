#include "seatstatustable.h"
#include "tableutil.h"
#include "palette.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QColor>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTimer>

SeatStatusTable::SeatStatusTable(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"状态", "车厢", "座位号", "旅客姓名", "身份证号"});
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &SeatStatusTable::selectionChanged);
}

void SeatStatusTable::setTrain(Train *train)
{
    m_train = train;
    rebuild();
}

void SeatStatusTable::setSellMode(bool sell)
{
    m_sell = sell;
    rebuild();
}

void SeatStatusTable::rebuild()
{
    m_table->setRowCount(0);
    m_weights.clear();
    if (!m_train)
        return;
    int row = 0;
    for (int c = 1; c <= m_train->carriages(); ++c) {
        const int classType = m_train->carriageClass(c);
        for (int s = 1; s <= m_train->seatsPerCarriage(); ++s) {
            const bool occupied = m_train->isSeatOccupied(c, s);
            const bool operable = (m_sell ? !occupied : occupied);
            m_table->insertRow(row);

            // 状态色块列：居中显示圆角方形，而非全格填充
            auto *stateItem = new QTableWidgetItem;
            // 可操作/不可操作都显示状态色块（■），区别只是是否可选中
            if (occupied)
                stateItem->setForeground(Palette::kDanger); // 红色，已售
            else
                stateItem->setForeground(Palette::kSuccess); // 绿色，空座
            stateItem->setText("■");
            stateItem->setTextAlignment(Qt::AlignCenter);

            // 背景色：不可操作浅灰，可操作一等座浅灰蓝底，可操作二等座白底
            if (!operable)
                stateItem->setBackground(Palette::kBorder);
            else if (classType == 1)
                stateItem->setBackground(Palette::kTableAlt);
            else
                stateItem->setBackground(Palette::kWhite);

            // 统一放大方块大小，确保所有色块一致
            stateItem->setFont(QFont(stateItem->font().family(), 16));
            stateItem->setFlags(Qt::ItemIsEnabled);
            m_table->setItem(row, 0, stateItem);
            // 行高：容纳放大后的方块
            m_table->setRowHeight(row, 30);

            auto *cItem = new QTableWidgetItem(QString::number(c));
            cItem->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, 1, cItem);
            auto *sItem = new QTableWidgetItem(QString::number(s));
            sItem->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, 2, sItem);
            if (occupied) {
                const Seat &seat = m_train->seatAt(c, s);
                auto *nItem = new QTableWidgetItem(seat.name());
                nItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(row, 3, nItem);
                auto *idItem = new QTableWidgetItem(seat.id());
                idItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(row, 4, idItem);
            } else {
                auto *nItem = new QTableWidgetItem("-");
                nItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(row, 3, nItem);
                auto *idItem = new QTableWidgetItem("-");
                idItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(row, 4, idItem);
            }

            // 不可操作行：整行标灰且不可选中
            if (!operable) {
                for (int col = 0; col < 5; ++col) {
                    QTableWidgetItem *it = m_table->item(row, col);
                    it->setFlags(it->flags() & ~Qt::ItemIsSelectable);
                    it->setForeground(Palette::kTextGhost);
                }
            }
            ++row;
        }
    }
    // 计算各列内容权重（按最长内容宽度），显示后按权重填满可用宽度
    computeColumnWeights(m_table, m_weights);
    applyColumnWidths();
}

void SeatStatusTable::applyColumnWidths()
{
    applyWeightedColumnWidths(m_table, m_weights);
}

void SeatStatusTable::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 等事件循环完成布局后再分配列宽，避免视口宽度尚未就绪
    QTimer::singleShot(0, this, &SeatStatusTable::applyColumnWidths);
}

void SeatStatusTable::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyColumnWidths();
}

int SeatStatusTable::selectedCarriage() const
{
    const int row = m_table->currentRow();
    if (row < 0)
        return 0;
    return m_table->item(row, 1)->text().toInt();
}

int SeatStatusTable::selectedSeat() const
{
    const int row = m_table->currentRow();
    if (row < 0)
        return 0;
    return m_table->item(row, 2)->text().toInt();
}
