#ifndef TRAINLISTVIEW_H
#define TRAINLISTVIEW_H

#include <QWidget>
#include <QVector>
#include "train.h"

class QLabel;
class QLineEdit;
class QTableWidget;
class QFrame;
class QStackedLayout;

// 班次列表组件：标题 + 搜索框 + 班次表格 + 空状态引导（置于表格框内）
class TrainListView : public QWidget
{
    Q_OBJECT
public:
    explicit TrainListView(QWidget *parent = nullptr);

    void setTrains(const QVector<Train> &trains); // 刷新数据（含过滤）
    QString currentTrainNo() const;               // 当前选中班次号，无选中返回空
    void clearSearch();

signals:
    void trainSelected(const QString &no);       // 选中变化（含过滤后清空）
    void trainDoubleClicked(const QString &no);  // 双击行 → 直接售票

private slots:
    void applyFilter();
    void onSelectionChanged();
    void onDoubleClicked(int row, int column);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyColumnWidths();

    QLabel *m_title;
    QLineEdit *m_searchEdit;
    QTableWidget *m_table;
    QFrame *m_emptyFrame;
    QLabel *m_emptyIcon;
    QLabel *m_emptyHint;
    QStackedLayout *m_stack;
    QVector<Train> m_trains;
    QVector<int> m_weights; // 各列内容权重（数据刷新时计算）
    int m_totalWeight = 0;
};

#endif // TRAINLISTVIEW_H
