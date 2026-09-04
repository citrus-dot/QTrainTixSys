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

// 自定义 QPainter 环形图控件
class DonutChart : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animationProgress READ animationProgress WRITE setAnimationProgress)
public:
    explicit DonutChart(QWidget *parent = nullptr);

    void setSlices(const QVector<DonutSlice> &slices);
    void setTitle(const QString &title);
    void animate();

    int animationProgress() const { return m_animProgress; }
    void setAnimationProgress(int p);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<DonutSlice> m_slices;
    QString m_title;
    int m_animProgress = 100;
};

#endif // DONUTCHART_H