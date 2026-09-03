#ifndef TRAINLISTVIEW_H
#define TRAINLISTVIEW_H

#include <QWidget>
#include <QVector>
#include "train.h"

class QLabel;
class QLineEdit;
class QTableWidget;

// 班次列表组件：标题 + 搜索框 + 班次表格 + 空状态引导
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

private:
    QLabel *m_title;
    QLabel *m_emptyHint;
    QLineEdit *m_searchEdit;
    QTableWidget *m_table;
    QVector<Train> m_trains;
};

#endif // TRAINLISTVIEW_H
