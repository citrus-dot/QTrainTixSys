#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>

class QButtonGroup;
class QPushButton;

// 左侧导航栏：品牌区 + 页面导航 + 底部关于
class SidebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    void setCurrentPage(int index);
    int currentPage() const;

signals:
    void pageSelected(int index);
    void aboutClicked();

private:
    QButtonGroup *m_group;
    QList<QPushButton *> m_navButtons;
};

#endif // SIDEBARWIDGET_H