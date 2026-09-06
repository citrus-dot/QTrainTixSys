#include "donutchart.h"
#include "palette.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

DonutChart::DonutChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(240, 220);
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true); // 无按键也接收 mouseMove，用于悬停高亮
}

void DonutChart::setSlices(const QVector<DonutSlice> &slices)
{
    m_slices = slices;
    m_hoverIndex = -1;
    m_hoverFrom.clear();
    m_hoverTo.clear();
    m_hoverProgress = 0.0;
    // 不动 progress：动画节奏完全由 animate() 控制，避免先画终态再归零的闪帧
    update();
}

void DonutChart::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void DonutChart::setNote(const QString &note)
{
    m_note = note;
    update();
}

void DonutChart::animate()
{
    // 停掉可能仍在运行的旧动画，避免多个动画同时驱动同一属性造成回跳
    const auto oldAnims = findChildren<QPropertyAnimation *>();
    for (auto *a : oldAnims)
        a->stop();
    auto *anim = new QPropertyAnimation(this, "animationProgress", this);
    anim->setDuration(800);
    anim->setStartValue(0.0);
    anim->setEndValue(100.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DonutChart::setAnimationProgress(double p)
{
    m_animProgress = p;
    update();
}

void DonutChart::setHoverProgress(double p)
{
    m_hoverProgress = p;
    update();
}

// 第 i 段当前加粗强度：动画起点/终点向量的插值（OutBack 过冲时强度可短暂 >1 或 <0，
// 表现为先压细再弹粗的回弹感，由调用方 qRound 自然收敛）
double DonutChart::hoverStrength(int i) const
{
    if (i < 0 || i >= m_hoverFrom.size() || i >= m_hoverTo.size())
        return 0.0;
    return m_hoverFrom[i] + (m_hoverTo[i] - m_hoverFrom[i]) * m_hoverProgress;
}

QVector<double> DonutChart::currentStrengths() const
{
    QVector<double> cur(m_slices.size());
    for (int i = 0; i < cur.size(); ++i)
        cur[i] = hoverStrength(i);
    return cur;
}

// 悬停加粗的弹性动画：把「当前各段强度」固化为起点，目标段缓动到 1、
// 其余到 0——悬停从一段滑到另一段时，旧段平滑回缩、新段弹性弹出，均无跳变
void DonutChart::animateHoverTo(int targetIndex)
{
    const auto oldAnims = findChildren<QPropertyAnimation *>();
    for (auto *a : oldAnims)
        if (a->propertyName() == "hoverProgress")
            a->stop();

    m_hoverFrom = currentStrengths();
    m_hoverTo = QVector<double>(m_slices.size(), 0.0);
    if (targetIndex >= 0 && targetIndex < m_hoverTo.size())
        m_hoverTo[targetIndex] = 1.0;
    m_hoverProgress = 0.0;

    auto *anim = new QPropertyAnimation(this, "hoverProgress", this);
    anim->setDuration(420);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

DonutChart::ChartGeometry DonutChart::chartGeometry() const
{
    ChartGeometry g;
    g.top = 6;
    if (!m_title.isEmpty())
        g.top += 20;
    if (!m_note.isEmpty())
        g.top += 16;
    const int chartSize = qMax(qMin(width() - 150, height() - g.top - 10), 110);
    g.cx = width() / 2;
    g.cy = g.top + (height() - g.top) / 2;
    g.penWidth = chartSize / 4;
    g.radius = (chartSize - g.penWidth) / 2;
    return g;
}

int DonutChart::sliceAt(const QPoint &pos) const
{
    if (m_slices.isEmpty() || m_animProgress < 100.0)
        return -1; // 动画期间不响应悬停，避免与生长中的弧段错位

    const ChartGeometry g = chartGeometry();
    const double dx = pos.x() - g.cx;
    const double dy = g.cy - pos.y(); // 屏幕坐标 → 数学坐标（y 向上）
    const double dist = std::hypot(dx, dy);
    // 命中带：弧心线两侧各留 2px 余量
    if (dist < g.radius - g.penWidth / 2.0 - 2 || dist > g.radius + g.penWidth / 2.0 + 2)
        return -1;

    // Qt 角度：逆时针为正、0° = 3 点方向；切片按顺时针（角度递减）依次排列
    double deg = qRadiansToDegrees(qAtan2(dy, dx));
    while (deg > 270.0)
        deg -= 360.0;
    while (deg <= -90.0)
        deg += 360.0;

    double total = 0;
    for (const auto &s : m_slices)
        total += s.value;
    if (total <= 0)
        return -1;

    double start = 270.0;
    for (int i = 0; i < m_slices.size(); ++i) {
        const double full = 360.0 * m_slices[i].value / total;
        if (m_slices[i].value > 0 && deg <= start + 1e-9 && deg > start - full)
            return i;
        start -= full;
    }
    return -1; // 落在缺口（浮点误差兜底）
}

void DonutChart::mouseMoveEvent(QMouseEvent *event)
{
    const int idx = sliceAt(event->pos());
    if (idx != m_hoverIndex) {
        m_hoverIndex = idx;
        animateHoverTo(idx);
    }
    if (idx >= 0) {
        double total = 0;
        for (const auto &s : m_slices)
            total += s.value;
        const DonutSlice &s = m_slices[idx];
        QToolTip::showText(event->globalPosition().toPoint(),
                           QString("%1：%2（%3%）")
                               .arg(s.label)
                               .arg(static_cast<int>(s.value))
                               .arg(qRound(100.0 * s.value / total)),
                           this);
    }
    QWidget::mouseMoveEvent(event);
}

void DonutChart::leaveEvent(QEvent *event)
{
    if (m_hoverIndex != -1) {
        m_hoverIndex = -1;
        animateHoverTo(-1);
    }
    QWidget::leaveEvent(event);
}

void DonutChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 标题 + 注释行（总班次等汇总信息）；与 chartGeometry() 的 top 累计方式一致
    const ChartGeometry g = chartGeometry();
    int top = 6;
    if (!m_title.isEmpty()) {
        QFont f = p.font();
        f.setPointSize(10);
        f.setBold(true);
        p.setFont(f);
        p.setPen(Palette::kText);
        p.drawText(QRect(0, top, width(), 20), Qt::AlignCenter, m_title);
        top += 20;
    }
    if (!m_note.isEmpty()) {
        QFont f = p.font();
        f.setPointSize(9);
        f.setBold(false);
        p.setFont(f);
        p.setPen(Palette::kTextFaint);
        p.drawText(QRect(0, top, width(), 16), Qt::AlignCenter, m_note);
    }

    const int cx = g.cx, cy = g.cy;
    const int penWidth = g.penWidth, radius = g.radius;

    double total = 0;
    for (const auto &s : m_slices)
        total += s.value;
    if (total <= 0) {
        // 空状态：灰环
        p.setPen(QPen(Palette::kBorder, penWidth));
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRect(cx - radius, cy - radius, radius * 2, radius * 2), 0, 360 * 16);
        return;
    }

    const QRectF rect(cx - radius, cy - radius, radius * 2, radius * 2);

    // 从 6 点方向起、顺时针依次展开：首段(绿)在前，末段(红)接续收尾回起点。
    // 最终分布先行预计算：末段终点固定在「越过起点 kExt」的位置（红帽回压绿段），
    // 动画只是让末段末端以近似匀速收敛到该预计算终点——终点即最终分布，无跳变：
    //  1) 基础弧：各段按已揭示量绘制，首尾相接于移动前沿；
    //  2) 末段末端延伸量随末段展开线性增长（0 → kExt 铺满整个末段展开期），
    //     红端全程匀速滑向预计算终点，越过 6 点接缝不是尾段专属动作；
    //  3) 首段末端补画 stub：在首末段交接处沿顺时针压过末段起点（绿在上），
    //     压过量随末段揭示长度增长，逆时针侧始终夹在已画前沿内，全程无缝。
    const int n = m_slices.size();
    const double kStart = 270.0; // 起始角 270° = 6 点方向
    const double kExt = 10.0;    // 交接处最终重叠角（预计算的最终分布）
    const double totalSweep = 360.0 * m_animProgress / 100.0;

    QVector<double> starts(n), draws(n), fulls(n);
    double revealed = 0.0;
    for (int i = 0; i < n; ++i) {
        fulls[i] = 360.0 * m_slices[i].value / total;
        starts[i] = kStart - revealed;
        draws[i] = qBound(0.0, totalSweep - revealed, fulls[i]);
        revealed += fulls[i];
    }
    p.setBrush(Qt::NoBrush);
    const auto drawArcSeg = [&](int i, double extra, int pw) {
        if (draws[i] + extra <= 0.01)
            return;
        p.setPen(QPen(m_slices[i].color, pw, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(rect, qRound(starts[i] * 16), qRound(-(draws[i] + extra) * 16));
    };
    // 末段末端延伸量：随末段展开进度线性增长，终点 = 预计算的最终重叠 kExt
    const double revBeforeLast = revealed - fulls[n - 1];
    const auto extOf = [&]() {
        return kExt * qBound(0.0, (totalSweep - revBeforeLast) / fulls[n - 1], 1.0);
    };
    // 各段加粗宽度：常规 penWidth，悬停段按弹性强度最高 +5px
    const auto hoverW = [&](int i) { return penWidth + qRound(5.0 * hoverStrength(i)); };
    for (int i = 0; i < n; ++i) {
        if (hoverStrength(i) > 0.01)
            continue; // 有悬停强度的段（含淡出中的旧段）最后加粗画在最上层
        drawArcSeg(i, i == n - 1 ? extOf() : 0.0, penWidth);
    }
    // 首段在首末交接处的补画（末段起点之上）：绿帽压过红段起点，
    // 压入量随红段揭示长度增长——红段从绿帽下方逐渐探出，最终稳定为 kExt
    // （首段悬停加粗时整段最后重画，补画让位避免接头瑕疵）
    if (n > 1 && draws[1] > 0 && hoverStrength(0) <= 0.01) {
        const double jA = starts[1];                     // 交接点
        const double frontier = kStart - totalSweep;     // 当前前沿
        const double blend = 3.0;                        // 与首段本体的融合余量
        const double ccw = jA + blend;                   // 逆时针侧埋入首段本体
        const double cw = qMax(jA - kExt, qMin(frontier, jA)); // 顺时针侧不超过已揭示前沿
        if (ccw > cw) {
            p.setPen(QPen(m_slices[0].color, penWidth, Qt::SolidLine, Qt::RoundCap));
            p.drawArc(rect, qRound(ccw * 16), qRound(-(ccw - cw) * 16));
        }
    }
    // 悬停段（含淡出中的旧段）：按各自弹性强度加粗、最后绘制盖在相邻段上，
    // 形成"弹出/回缩"的平滑过渡
    for (int i = 0; i < n; ++i)
        if (hoverStrength(i) > 0.01)
            drawArcSeg(i, i == n - 1 ? extOf() : 0.0, hoverW(i));

    // 记录引出线几何：色段中点，按完整数据计算（动画期间保持稳定）
    struct Callout { const DonutSlice *slice; double midDeg; };
    QVector<Callout> callouts;
    double startAngle = 270.0; // 起始角 270° = 6 点方向，与圆弧绘制一致
    for (const auto &s : m_slices) {
        if (s.value > 0)
            callouts.append({&s, startAngle - 180.0 * s.value / total});
        startAngle -= 360.0 * s.value / total;
    }

    // 中心：总座位容量
    QFont cf = p.font();
    cf.setPointSize(15);
    cf.setBold(true);
    p.setFont(cf);
    p.setPen(Palette::kText);
    p.drawText(QRect(cx - radius, cy - 20, radius * 2, 24), Qt::AlignCenter,
               QString::number(static_cast<int>(total)));
    QFont sf = p.font();
    sf.setPointSize(8);
    sf.setBold(false);
    p.setFont(sf);
    p.setPen(Palette::kTextFaint);
    p.drawText(QRect(cx - radius, cy + 3, radius * 2, 14), Qt::AlignCenter, "总座位");

    // 引出线注释：灰线严格沿色段中点的径向射线引出，至文字行水平线处的折点
    // （折点自然落在最外侧），再转横线接入文字，两线呈钝角
    const QFont labelFont(p.font().family(), 9);
    p.setFont(labelFont);
    const QFontMetrics fm(labelFont);
    const QColor lineColor = Palette::kDonutCallout;
    const QColor textColor = Palette::kTextMuted;
    const double r0 = radius + penWidth / 2.0 + 2;
    const int pad = 8;
    for (const auto &c : callouts) {
        const QString text = QString("%1 %2（%3%）")
                                 .arg(c.slice->label)
                                 .arg(static_cast<int>(c.slice->value))
                                 .arg(qRound(100.0 * c.slice->value / total));
        const int tw = fm.horizontalAdvance(text);
        const int th = fm.height();

        // 按中点方位选最近的角放文字
        const double rad = qDegreesToRadians(c.midDeg);
        const double cosv = qCos(rad), sinv = qSin(rad);
        const QPointF p0(cx + r0 * cosv, cy - r0 * sinv);
        const bool rightSide = cosv >= 0;
        const bool upper = sinv >= 0;
        int tx, tyLine; // tyLine: 期望的文字行中线 y
        if (rightSide && upper) {                       // 右上
            tx = width() - tw - pad;
            tyLine = pad + th / 2;
        } else if (!rightSide && upper) {               // 左上
            tx = pad;
            tyLine = pad + th / 2;
        } else if (!rightSide && !upper) {              // 左下
            tx = pad;
            tyLine = height() - pad - th / 2;
        } else {                                        // 右下
            tx = width() - tw - pad;
            tyLine = height() - pad - th / 2;
        }

        // 折点 = 径向射线与文字行水平线的交点（尽量靠外）
        double t = (qAbs(sinv) > 1e-6) ? (cy - tyLine) / sinv : r0;
        t = qMax(t, r0);
        double ex = cx + t * cosv;
        double elbowY = cy - t * sinv;
        // 横线从折点连到文字边缘；若折点越过了文字边缘，收敛到文字边缘外侧
        const double xTextEdge = rightSide ? tx - 6.0 : tx + tw + 6.0;
        if (rightSide && ex > xTextEdge - 16.0)
            ex = xTextEdge - 16.0;
        if (!rightSide && ex < xTextEdge + 16.0)
            ex = xTextEdge + 16.0;
        ex = qBound(static_cast<double>(pad), ex, static_cast<double>(width() - pad));

        const QPointF elbow(ex, elbowY);
        p.setPen(QPen(lineColor, 1));
        p.drawLine(p0, elbow);
        p.drawLine(elbow, QPointF(xTextEdge, elbowY));

        // 文字垂直居中于横线
        const int ty = qBound(pad, static_cast<int>(elbowY - th / 2.0), height() - th - pad);
        p.setPen(textColor);
        p.drawText(QRect(tx, ty, tw, th),
                   Qt::AlignVCenter | (rightSide ? Qt::AlignRight : Qt::AlignLeft), text);
    }
}
