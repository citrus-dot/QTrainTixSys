#include "trainlistview.h"
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QFrame>
#include <QIcon>

TrainListView::TrainListView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_title = new QLabel("班次列表", this);
    m_title->setObjectName("panelTitle");
    layout->addWidget(m_title);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索班次号 / 发车城市 / 终点城市");
    m_searchEdit->setClearButtonEnabled(true);
    layout->addWidget(m_searchEdit);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"班次号", "发车日期", "发车时间", "发车城市", "终点城市", "车厢数", "每厢座位数", "余票"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    // 空状态引导：置于表格框内（与表格同尺寸切换）
    m_emptyFrame = new QFrame(this);
    m_emptyFrame->setObjectName("emptyFrame");
    auto *emptyLayout = new QVBoxLayout(m_emptyFrame);
    emptyLayout->setSpacing(10);
    m_emptyIcon = new QLabel(m_emptyFrame);
    m_emptyIcon->setPixmap(QIcon(":/icons/nav-train.svg").pixmap(44, 44));
    m_emptyIcon->setAlignment(Qt::AlignCenter);
    m_emptyIcon->setProperty("emptyIcon", true);
    emptyLayout->addStretch(1);
    emptyLayout->addWidget(m_emptyIcon);
    m_emptyHint = new QLabel(m_emptyFrame);
    m_emptyHint->setObjectName("emptyHint");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_emptyHint->setWordWrap(true);
    emptyLayout->addWidget(m_emptyHint);
    emptyLayout->addStretch(2);

    m_stack = new QStackedLayout;
    m_stack->addWidget(m_table);
    m_stack->addWidget(m_emptyFrame);
    layout->addLayout(m_stack, 1);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &TrainListView::applyFilter);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &TrainListView::onSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &TrainListView::onDoubleClicked);
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
    // 空状态引导：无数据时在表格框内显示提示
    const bool empty = m_table->rowCount() == 0;
    m_stack->setCurrentWidget(empty ? m_emptyFrame : m_table);
    if (empty)
        m_emptyHint->setText(m_trains.isEmpty()
            ? "暂无班次数据\n点击上方「新增班次」或先「打开」数据文件"
            : "没有匹配的班次\n请更换搜索关键词");
}

void TrainListView::onSelectionChanged()
{
    emit trainSelected(currentTrainNo());
}

void TrainListView::onDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QTableWidgetItem *item = m_table->item(row, 0);
    if (item)
        emit trainDoubleClicked(item->data(Qt::UserRole).toString());
}
