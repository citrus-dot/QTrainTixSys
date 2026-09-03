#include "trainlistview.h"
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>

TrainListView::TrainListView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("输入班次号/发车城市/终点城市筛选");
    m_searchEdit->setClearButtonEnabled(true);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"班次号", "发车日期", "发车时间", "发车城市", "终点城市", "车厢数", "每厢座位数", "余票"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(m_searchEdit);
    layout->addWidget(m_table);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &TrainListView::applyFilter);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &TrainListView::onSelectionChanged);
}

void TrainListView::setTrains(const QVector<Train> &trains)
{
    m_trains = trains;
    applyFilter();
}

QString TrainListView::currentTrainNo() const
{
    int row = m_table->currentRow();
    if (row < 0)
        return QString();
    QTableWidgetItem *item = m_table->item(row, 0);
    return item ? item->data(Qt::UserRole).toString() : QString();
}

void TrainListView::clearSearch()
{
    m_searchEdit->clear();
}

void TrainListView::applyFilter()
{
    const QString keyword = m_searchEdit->text().trimmed();
    const QString prevNo = currentTrainNo();
    m_table->setRowCount(0);
    int row = 0;
    for (const Train &t : m_trains) {
        if (!keyword.isEmpty()
            && !t.no().contains(keyword, Qt::CaseInsensitive)
            && !t.from().contains(keyword, Qt::CaseInsensitive)
            && !t.to().contains(keyword, Qt::CaseInsensitive))
            continue;
        QTableWidgetItem *noItem = new QTableWidgetItem(t.no());
        noItem->setData(Qt::UserRole, t.no());
        m_table->insertRow(row);
        m_table->setItem(row, 0, noItem);
        m_table->setItem(row, 1, new QTableWidgetItem(t.date()));
        m_table->setItem(row, 2, new QTableWidgetItem(t.departTime()));
        m_table->setItem(row, 3, new QTableWidgetItem(t.from()));
        m_table->setItem(row, 4, new QTableWidgetItem(t.to()));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(t.carriages())));
        m_table->setItem(row, 6, new QTableWidgetItem(QString::number(t.seatsPerCarriage())));
        m_table->setItem(row, 7, new QTableWidgetItem(QString::number(t.remainingSeats())));
        ++row;
    }
    // 恢复之前选中的班次（若仍在过滤结果中）
    if (!prevNo.isEmpty()) {
        for (int r = 0; r < m_table->rowCount(); ++r) {
            if (m_table->item(r, 0)->data(Qt::UserRole).toString() == prevNo) {
                m_table->selectRow(r);
                break;
            }
        }
    }
}

void TrainListView::onSelectionChanged()
{
    emit trainSelected(currentTrainNo());
}
