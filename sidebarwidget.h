#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>

class QButtonGroup;
class QPushButton;
class QLabel;

// 左侧导航栏 v0.2.0：大图标垂直按钮 + 底部统计信息
class SidebarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

    void setCurrentPage(int index);
    int currentPage() const;

    void updateStats(int trainCount, int remainingSeats, int soldSeats);

signals:
    void pageSelected(int index);
    void aboutClicked();

private:
    QButtonGroup *m_group;
    QLabel *m_statsLabel;
};

#endif // SIDEBARWIDGET_H