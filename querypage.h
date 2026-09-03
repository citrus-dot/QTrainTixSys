#ifndef QUERYPAGE_H
#define QUERYPAGE_H

#include <QWidget>
#include "trainsystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QueryPage; }
QT_END_NAMESPACE

// 查询页：选择班次查看余票、可选座位与停靠站
class QueryPage : public QWidget
{
    Q_OBJECT
public:
    explicit QueryPage(QWidget *parent = nullptr);
    ~QueryPage();

    void setSystem(const TrainSystem *system);
    void refresh(); // 重新加载班次列表并刷新显示

private slots:
    void onTrainChanged();

private:
    Ui::QueryPage *ui;
    const TrainSystem *m_system = nullptr;
};

#endif // QUERYPAGE_H