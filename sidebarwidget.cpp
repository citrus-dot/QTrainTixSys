#include "sidebarwidget.h"
#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QStyle>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("sidebar");
    setFixedWidth(188);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 16, 12, 12);
    layout->setSpacing(4);

    auto *brand = new QLabel(this);
    brand->setObjectName("sidebarBrand");
    brand->setPixmap(QIcon(":/icons/nav-train.svg").pixmap(20, 20));
    brand->setText("  列车售票系统");
    brand->setContentsMargins(4, 0, 0, 10);
    layout->addWidget(brand);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    auto addNav = [&](const QString &text, const QString &icon, int id) {
        auto *btn = new QPushButton(QIcon(icon), text, this);
        btn->setProperty("nav", true);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIconSize(QSize(18, 18));
        m_group->addButton(btn, id);
        m_navButtons.append(btn);
        layout->addWidget(btn);
    };

    addNav("班次管理", ":/icons/nav-train.svg", 0);
    addNav("查询", ":/icons/nav-search.svg", 1);

    layout->addStretch();

    auto *aboutBtn = new QPushButton(QIcon(":/icons/about.svg"), "关于", this);
    aboutBtn->setProperty("nav", true);
    aboutBtn->setCursor(Qt::PointingHandCursor);
    aboutBtn->setIconSize(QSize(18, 18));
    layout->addWidget(aboutBtn);
    connect(aboutBtn, &QPushButton::clicked, this, &SidebarWidget::aboutClicked);

    connect(m_group, &QButtonGroup::idClicked, this, &SidebarWidget::pageSelected);
}

void SidebarWidget::setCurrentPage(int index)
{
    if (auto *btn = m_group->button(index))
        btn->setChecked(true);
}

int SidebarWidget::currentPage() const
{
    return m_group->checkedId();
}