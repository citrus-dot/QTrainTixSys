#include "statisticspage.h"
#include "donutchart.h"
#include "tableutil.h"
#include "palette.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTimer>

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
        // 量程 0-1000（0.1% 分辨率）：value 是 int，量程 100 时 OutCubic 尾段
        // 每帧增量 <1 被量化，最后一帧会从 ~97 跳到 100（一格 3px，肉眼可见）；
        // 1000 量程下量化步进约 0.3px，动画收尾平滑
        outBar->setRange(0, 1000);
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
    m_trainTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    tl->addWidget(m_trainTable, 1);
    layout->addWidget(tableCard, 2);
}

StatisticsPage::ClassStats StatisticsPage::calcClassStats(const QVector<Train> &trains)
{
    ClassStats stats = {};
    for (const Train &t : trains) {
        for (int c = 1; c <= t.carriages(); ++c) {
            if (t.carriageClass(c) == 1) {
                ++stats.firstCarriages;
                stats.firstSeats += t.seatsPerCarriage();
            } else {
                ++stats.secondCarriages;
                stats.secondSeats += t.seatsPerCarriage();
            }
        }
    }
    return stats;
}

void StatisticsPage::animateBarTo(QProgressBar *bar, int target)
{
    // 停掉该进度条上仍在运行的旧动画，避免多个动画抢占同一属性
    const auto oldAnims = bar->findChildren<QPropertyAnimation *>();
    for (auto *a : oldAnims)
        a->stop();
    if (target <= 0) {
        bar->setValue(0);
        return;
    }
    // bar 量程 0-1000，target 是百分比，动画值同比例放大 10 倍
    auto *anim = new QPropertyAnimation(bar, "value", bar);
    anim->setDuration(600);
    anim->setStartValue(0);
    anim->setEndValue(target * 10);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
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

    // 更新环形图：概览数据以注释形式并入图中
    // 标题下注释行显示总班次数，环心显示总座位容量，引出线标注已售/余票
    m_donutChart->setNote(trains.isEmpty() ? QString("暂无数据")
                                           : QString("总班次 %1").arg(trains.size()));
    // 环形图：从 6 点方向起顺时针展开，余票(绿)在前、已售(红)接续收尾回起点
    // 已售淡红 / 余票淡绿；注释线指向各自色段中点，文字就近放四角
    QVector<DonutSlice> slices;
    if (totalCap > 0) {
        slices.append({"余票", static_cast<double>(remaining), Palette::kDonutRemain});
        slices.append({"已售", static_cast<double>(sold), Palette::kDonutSold});
    }
    m_donutChart->setSlices(slices);
    m_donutChart->setTitle("余票分布");
    m_donutChart->animate();

    // 更新等级分布（进度条从 0 入场动画到目标占比）
    ClassStats cs = calcClassStats(trains);
    const int totalSeats = cs.firstSeats + cs.secondSeats;
    if (totalSeats > 0) {
        const int fp = static_cast<int>(100.0 * cs.firstSeats / totalSeats);
        const int sp = 100 - fp;
        m_firstLabel->setText(QString("一等座 %1 座 (%2%)").arg(cs.firstSeats).arg(fp));
        m_secondLabel->setText(QString("二等座 %1 座 (%2%)").arg(cs.secondSeats).arg(sp));
        animateBarTo(m_firstProgress, fp);
        animateBarTo(m_secondProgress, sp);
    } else {
        m_firstLabel->setText("一等座 0 座 (0%)");
        m_secondLabel->setText("二等座 0 座 (0%)");
        animateBarTo(m_firstProgress, 0);
        animateBarTo(m_secondProgress, 0);
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

        auto *rateItem = makeCenteredItem(QString("%1%").arg(rate, 0, 'f', 1));
        // 上座率超50% 标红提示
        rateItem->setForeground(rate > 50 ? Palette::kDanger : Palette::kSuccess);

        m_trainTable->setItem(row, 0, makeCenteredItem(t.no()));
        m_trainTable->setItem(row, 1, makeCenteredItem(QString("%1 → %2").arg(t.from(), t.to())));
        m_trainTable->setItem(row, 2, makeCenteredItem(QString::number(t.carriages())));
        m_trainTable->setItem(row, 3, makeCenteredItem(QString::number(trainCap)));
        m_trainTable->setItem(row, 4, makeCenteredItem(QString::number(trainRemaining)));
        m_trainTable->setItem(row, 5, makeCenteredItem(QString::number(trainSold)));
        m_trainTable->setItem(row, 6, rateItem);
    }

    // 各列内容权重（按最长内容宽度），按权重填满可用宽度
    computeColumnWeights(m_trainTable, m_weights);
    applyColumnWidths();

    // 空状态
    if (trains.isEmpty()) {
        m_trainTable->setRowCount(1);
        m_trainTable->setSpan(0, 0, 1, 7);
        auto *empty = new QTableWidgetItem("暂无数据，请先打开或创建数据文件");
        empty->setTextAlignment(Qt::AlignCenter);
        empty->setForeground(Palette::kTextGhost);
        m_trainTable->setItem(0, 0, empty);
    }
}

void StatisticsPage::applyColumnWidths()
{
    applyWeightedColumnWidths(m_trainTable, m_weights);
}

void StatisticsPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 等事件循环完成布局后再分配列宽，避免视口宽度尚未就绪
    QTimer::singleShot(0, this, &StatisticsPage::applyColumnWidths);
}

void StatisticsPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    applyColumnWidths();
}