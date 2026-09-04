#include "statisticspage.h"
#include "donutchart.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QIcon>

StatisticsPage::StatisticsPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("statsPage");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    // 标题
    auto *title = new QLabel("数据统计", this);
    title->setObjectName("pageTitle");
    layout->addWidget(title);

    // === 三张概览卡片 ===
    auto *cardRow = new QHBoxLayout;
    cardRow->setSpacing(16);

    auto makeCard = [&](const QString &label, const QString &accent) -> QWidget * {
        auto *card = new QFrame(this);
        card->setObjectName("statCard");
        card->setProperty("accent", accent);
        card->setFixedHeight(110);
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 12, 16, 12);
        cl->setSpacing(4);
        auto *val = new QLabel("0", card);
        val->setObjectName("statValue");
        val->setAlignment(Qt::AlignCenter);
        auto *lb = new QLabel(label, card);
        lb->setObjectName("statLabel");
        lb->setAlignment(Qt::AlignCenter);
        cl->addStretch();
        cl->addWidget(val);
        cl->addWidget(lb);
        cl->addStretch();
        return card;
    };

    QWidget *c1 = makeCard("总班次数", "blue");
    QWidget *c2 = makeCard("总座位容量", "green");
    QWidget *c3 = makeCard("已售座位", "red");
    m_trainCount = c1->findChild<QLabel *>("statValue");
    m_capacityLabel = c2->findChild<QLabel *>("statValue");
    m_soldLabel = c3->findChild<QLabel *>("statValue");
    cardRow->addWidget(c1);
    cardRow->addWidget(c2);
    cardRow->addWidget(c3);
    layout->addLayout(cardRow);

    // 余票卡片
    auto *remCard = new QFrame(this);
    remCard->setObjectName("statCard");
    remCard->setProperty("accent", "blue");
    remCard->setFixedHeight(60);
    auto *rl = new QHBoxLayout(remCard);
    rl->setContentsMargins(20, 0, 20, 0);
    auto *remLabel = new QLabel("当前余票：", remCard);
    remLabel->setObjectName("statLabel");
    m_remainingLabel = new QLabel("0", remCard);
    m_remainingLabel->setObjectName("statValue");
    rl->addWidget(remLabel);
    rl->addWidget(m_remainingLabel);
    rl->addStretch();
    layout->addWidget(remCard);

    // === 中段：环形图 + 等级分布（双栏） ===
    auto *midRow = new QHBoxLayout;
    midRow->setSpacing(16);

    // 左侧：环形图
    auto *donutCard = new QFrame(this);
    donutCard->setObjectName("infoCard");
    auto *dl = new QVBoxLayout(donutCard);
    dl->setContentsMargins(16, 12, 16, 12);
    m_donutChart = new DonutChart(this);
    m_donutChart->setMinimumSize(200, 220);
    dl->addWidget(m_donutChart, 1);
    midRow->addWidget(donutCard, 1);

    // 右侧：等级分布
    auto *classCard = new QFrame(this);
    classCard->setObjectName("infoCard");
    auto *cl2 = new QVBoxLayout(classCard);
    cl2->setContentsMargins(20, 16, 20, 16);
    cl2->setSpacing(12);

    auto *classTitle = new QLabel("座位等级分布", classCard);
    classTitle->setObjectName("panelTitle");
    cl2->addWidget(classTitle);

    auto makeBar = [&](const QString &text, QLabel *&outLabel, QProgressBar *&outBar, const char *accent) -> QWidget * {
        auto *w = new QWidget(classCard);
        auto *wl = new QVBoxLayout(w);
        wl->setContentsMargins(0, 0, 0, 0);
        wl->setSpacing(4);
        outLabel = new QLabel(text, w);
        outLabel->setObjectName("statLabel");
        outBar = new QProgressBar(w);
        outBar->setObjectName("classBar");
        outBar->setProperty("accent", accent);
        outBar->setRange(0, 100);
        outBar->setValue(0);
        outBar->setTextVisible(false);
        outBar->setFixedHeight(20);
        wl->addWidget(outLabel);
        wl->addWidget(outBar);
        return w;
    };

    cl2->addWidget(makeBar("一等座", m_firstLabel, m_firstProgress, "first"), 1);
    cl2->addWidget(makeBar("二等座", m_secondLabel, m_secondProgress, "second"), 1);
    cl2->addStretch();
    midRow->addWidget(classCard, 1);
    layout->addLayout(midRow);

    // === 底部：班次详情列表 ===
    auto *tableCard = new QFrame(this);
    tableCard->setObjectName("infoCard");
    auto *tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 12, 16, 12);
    tl->setSpacing(8);

    auto *tableTitle = new QLabel("班次详情", tableCard);
    tableTitle->setObjectName("panelTitle");
    tl->addWidget(tableTitle);

    m_trainTable = new QTableWidget(this);
    m_trainTable->setColumnCount(7);
    m_trainTable->setHorizontalHeaderLabels({"班次号", "始发站→终点站", "车厢数", "座位数", "余票", "已售", "上座率"});
    m_trainTable->verticalHeader()->setVisible(false);
    m_trainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_trainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_trainTable->setAlternatingRowColors(true);
    m_trainTable->horizontalHeader()->setStretchLastSection(true);
    m_trainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tl->addWidget(m_trainTable, 1);
    layout->addWidget(tableCard, 2);
}

StatisticsPage::ClassStats StatisticsPage::calcClassStats(const QVector<Train> &trains)
{
    ClassStats stats = {};
    for (const Train &t : trains) {
        // 假设每节车厢的前半部分是一等座，后半部分是二等座
        // 实际上 train.h 没有明确的等级区分，按车厢编号奇偶模拟
        if (t.carriages() % 2 == 0) {
            stats.firstCarriages += t.carriages() / 2;
            stats.secondCarriages += t.carriages() / 2;
        } else {
            stats.firstCarriages += t.carriages() / 2;
            stats.secondCarriages += t.carriages() / 2 + 1;
        }
    }
    stats.firstSeats = stats.firstCarriages * 50;    // 假设每车厢50座
    stats.secondSeats = stats.secondCarriages * 50;
    return stats;
}

void StatisticsPage::setData(const QVector<Train> &trains)
{
    // 计算总数据
    int totalCap = 0, sold = 0;
    for (const Train &t : trains) {
        totalCap += t.carriages() * t.seatsPerCarriage();
        sold += t.carriages() * t.seatsPerCarriage() - t.remainingSeats();
    }
    const int remaining = totalCap - sold;

    // 更新概览卡片
    m_trainCount->setText(QString::number(trains.size()));
    m_capacityLabel->setText(QString::number(totalCap));
    m_soldLabel->setText(QString::number(sold));
    m_remainingLabel->setText(QString::number(remaining));

    // 更新环形图
    QVector<DonutSlice> slices;
    if (totalCap > 0) {
        slices.append({"已售", static_cast<double>(sold), QColor("#E5484D")});
        slices.append({"余票", static_cast<double>(remaining), QColor("#2F6FED")});
    }
    m_donutChart->setSlices(slices);
    m_donutChart->setTitle("余票分布");
    m_donutChart->animate();

    // 更新等级分布
    ClassStats cs = calcClassStats(trains);
    const int totalSeats = cs.firstSeats + cs.secondSeats;
    if (totalSeats > 0) {
        const int fp = static_cast<int>(100.0 * cs.firstSeats / totalSeats);
        const int sp = 100 - fp;
        m_firstLabel->setText(QString("一等座 %1 座 (%2%)").arg(cs.firstSeats).arg(fp));
        m_secondLabel->setText(QString("二等座 %1 座 (%2%)").arg(cs.secondSeats).arg(sp));
        m_firstProgress->setValue(fp);
        m_secondProgress->setValue(sp);
    } else {
        m_firstLabel->setText("一等座 0 座 (0%)");
        m_secondLabel->setText("二等座 0 座 (0%)");
        m_firstProgress->setValue(0);
        m_secondProgress->setValue(0);
    }

    // 更新班次详情表格
    m_trainTable->setRowCount(0);
    for (const Train &t : trains) {
        const int row = m_trainTable->rowCount();
        m_trainTable->insertRow(row);
        const int trainCap = t.carriages() * t.seatsPerCarriage();
        const int trainRemaining = t.remainingSeats();
        const int trainSold = trainCap - trainRemaining;
        const double rate = trainCap > 0 ? 100.0 * trainSold / trainCap : 0;

        m_trainTable->setItem(row, 0, new QTableWidgetItem(t.no()));
        m_trainTable->setItem(row, 1, new QTableWidgetItem(QString("%1 → %2").arg(t.from(), t.to())));
        m_trainTable->setItem(row, 2, new QTableWidgetItem(QString::number(t.carriages())));
        m_trainTable->setItem(row, 3, new QTableWidgetItem(QString::number(trainCap)));
        m_trainTable->setItem(row, 4, new QTableWidgetItem(QString::number(trainRemaining)));
        m_trainTable->setItem(row, 5, new QTableWidgetItem(QString::number(trainSold)));
        m_trainTable->setItem(row, 6, new QTableWidgetItem(QString("%1%").arg(rate, 0, 'f', 1)));

        // 上座率超50% 标红提示
        if (rate > 50) {
            m_trainTable->item(row, 6)->setForeground(QColor("#E5484D"));
        } else {
            m_trainTable->item(row, 6)->setForeground(QColor("#1E7B45"));
        }
    }
    m_trainTable->resizeColumnsToContents();

    // 空状态
    if (trains.isEmpty()) {
        m_trainTable->setRowCount(1);
        m_trainTable->setSpan(0, 0, 1, 7);
        auto *empty = new QTableWidgetItem("暂无数据，请先打开或创建数据文件");
        empty->setTextAlignment(Qt::AlignCenter);
        empty->setForeground(QColor("#A8B0BF"));
        m_trainTable->setItem(0, 0, empty);
    }
}