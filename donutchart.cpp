#include "donutchart.h"
#include <QPainter>
#include <QPropertyAnimation>

DonutChart::DonutChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(180, 180);
    setAttribute(Qt::WA_StyledBackground, true);
}

void DonutChart::setSlices(const QVector<DonutSlice> &slices)
{
    m_slices = slices;
    m_animProgress = 100;
    update();
}

void DonutChart::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void DonutChart::animate()
{
    auto *anim = new QPropertyAnimation(this, "animationProgress", this);
    anim->setDuration(800);
    anim->setStartValue(0);
    anim->setEndValue(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void DonutChart::setAnimationProgress(int p)
{
    m_animProgress = p;
    update();
}

void DonutChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int side = qMin(width(), height());
    const int chartSize = side - 40;
    const int chartX = (width() - chartSize) / 2;
    const int chartY = (height() - chartSize) / 2 + 12;
    const int penWidth = chartSize / 4;
    const int radius = (chartSize - penWidth) / 2;

    // 绘制标题
    if (!m_title.isEmpty()) {
        p.setPen(QColor("#98A1B3"));
        QFont f = p.font();
        f.setPointSize(9);
        p.setFont(f);
        p.drawText(QRect(0, 0, width(), chartY), Qt::AlignCenter, m_title);
    }

    // 计算总和
    double total = 0;
    for (const auto &s : m_slices)
        total += s.value;
    if (total <= 0) {
        p.setPen(QColor("#E3E7EF"));
        p.drawArc(QRectF(chartX + penWidth / 2, chartY + penWidth / 2,
                         chartSize - penWidth, chartSize - penWidth),
                  0, 360 * 16);
        return;
    }

    // 绘制环形图各块
    const QRectF rect(chartX + penWidth / 2, chartY + penWidth / 2,
                      chartSize - penWidth, chartSize - penWidth);
    double startAngle = 90.0;
    for (const auto &s : m_slices) {
        const double sweep = 360.0 * s.value / total * m_animProgress / 100.0;
        p.setPen(QPen(s.color, penWidth, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(rect, static_cast<int>(-startAngle * 16),
                  static_cast<int>(-sweep * 16));
        startAngle += sweep;
    }

    // 中心文字（总数值）
    p.setPen(QColor("#1B2430"));
    QFont cf = p.font();
    cf.setPointSize(16);
    cf.setBold(true);
    p.setFont(cf);
    p.drawText(QRectF(chartX, chartY, chartSize, chartSize),
               Qt::AlignCenter, QString::number(static_cast<int>(total)));

    // 图例
    QFont lf = p.font();
    lf.setPointSize(10);
    p.setFont(lf);
    int legendY = chartY + chartSize + 8;
    int legendX = chartX;
    for (const auto &s : m_slices) {
        p.setPen(Qt::NoPen);
        p.setBrush(s.color);
        p.drawRoundedRect(legendX, legendY, 10, 10, 2, 2);
        p.setPen(QColor("#667085"));
        p.drawText(legendX + 16, legendY + 10,
                   QString("%1 (%2)").arg(s.label).arg(static_cast<int>(s.value)));
        legendY += 18;
    }
}