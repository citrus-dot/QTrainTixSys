#ifndef DONUTCHART_H
#define DONUTCHART_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QString>

struct DonutSlice {
    QString label;
    double value;
    QColor color;
};

// 自定义 QPainter 环形图控件（支持悬停高亮 + tooltip 明细）
class DonutChart : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double animationProgress READ animationProgress WRITE setAnimationProgress)
    Q_PROPERTY(double hoverProgress READ hoverProgress WRITE setHoverProgress)
public:
    explicit DonutChart(QWidget *parent = nullptr);

    void setSlices(const QVector<DonutSlice> &slices);
    void setTitle(const QString &title);
    void setNote(const QString &note); // 标题下的小字注释行（汇总信息）
    void animate();

    double animationProgress() const { return m_animProgress; }
    void setAnimationProgress(double p);
    double hoverProgress() const { return m_hoverProgress; }
    void setHoverProgress(double p);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    // 绘图几何（paintEvent 与 hit-test 共用，保证坐标系一致）
    struct ChartGeometry {
        int cx, cy;        // 圆心
        int penWidth;      // 弧宽
        int radius;        // 半径（至弧心线）
        int top;           // 标题+注释占用的顶部高度
    };
    ChartGeometry chartGeometry() const;
    // 屏幕坐标 → 切片下标；不在环上或动画未结束时返回 -1
    int sliceAt(const QPoint &pos) const;
    // 悬停加粗动画：目标段强度缓动到 1，其余缓动到 0（OutBack 弹性过冲）
    void animateHoverTo(int targetIndex);
    // 第 i 段当前加粗强度（0 = 常规，1 = 完全加粗），由 from/to 向量插值得出
    double hoverStrength(int i) const;
    QVector<double> currentStrengths() const;

    QVector<DonutSlice> m_slices;
    QString m_title;
    QString m_note;
    double m_animProgress = 100.0;
    double m_hoverProgress = 0.0;           // 悬停动画进度 t（0→1，强度 = from/to 插值）
    QVector<double> m_hoverFrom, m_hoverTo; // 各段加粗强度的动画起点/终点
    int m_hoverIndex = -1; // 当前悬停切片，-1 = 无
};

#endif // DONUTCHART_H
