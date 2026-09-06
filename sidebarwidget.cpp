#include "sidebarwidget.h"
#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QIcon>
#include <QStyle>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("sidebar");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(240);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 24, 16, 16);
    layout->setSpacing(4);

    // 品牌区
    auto *brandIcon = new QLabel(this);
    brandIcon->setPixmap(QIcon(":/icons/nav-train.svg").pixmap(36, 36));
    brandIcon->setAlignment(Qt::AlignCenter);
    layout->addWidget(brandIcon);

    auto *brand = new QLabel("列车售票系统", this);
    brand->setObjectName("sidebarBrand");
    brand->setAlignment(Qt::AlignCenter);
    layout->addWidget(brand);

    auto *subtitle = new QLabel("课程设计作业", this);
    subtitle->setObjectName("sidebarSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);

    layout->addSpacing(20);

    // 功能导航分组标题
    auto *navLabel = new QLabel("功能导航", this);
    navLabel->setObjectName("sidebarSection");
    layout->addWidget(navLabel);

    // 导航按钮组
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    struct NavItem { QString text; QString icon; int id; };
    const NavItem items[] = {
        {"班次管理", ":/icons/nav-train.svg", 0},
        {"查询",     ":/icons/nav-search.svg", 1},
        {"统计",     ":/icons/nav-chart.svg",  2},
    };

    for (const auto &ni : items) {
        auto *btn = new QPushButton(ni.text, this);
        btn->setProperty("nav", true);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(QIcon(ni.icon));
        btn->setIconSize(QSize(26, 26));
        btn->setFixedHeight(56);
        m_group->addButton(btn, ni.id);
        layout->addWidget(btn);
    }

    layout->addStretch();

    // 底部统计信息卡
    auto *infoCard = new QFrame(this);
    infoCard->setObjectName("sidebarInfo");
    auto *infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(14, 12, 14, 12);
    infoLayout->setSpacing(6);

    auto *infoTitle = new QLabel("数据概览", infoCard);
    infoTitle->setObjectName("sidebarInfoTitle");

    m_statsLabel = new QLabel("班次 0  ·  余票 0  ·  已售 0", infoCard);
    m_statsLabel->setObjectName("sidebarInfoSub");

    infoLayout->addWidget(infoTitle);
    infoLayout->addWidget(m_statsLabel);
    layout->addWidget(infoCard);

    layout->addSpacing(8);

    // 关于按钮 + 版本号
    auto *aboutBtn = new QPushButton(" 关于", this);
    aboutBtn->setProperty("nav", true);
    aboutBtn->setCursor(Qt::PointingHandCursor);
    aboutBtn->setIcon(QIcon(":/icons/about.svg"));
    aboutBtn->setIconSize(QSize(20, 20));
    aboutBtn->setFixedHeight(44);
    layout->addWidget(aboutBtn);
    connect(aboutBtn, &QPushButton::clicked, this, &SidebarWidget::aboutClicked);

    auto *version = new QLabel("版本号 v0.3.0", this);
    version->setObjectName("sidebarVersion");
    layout->addWidget(version);

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

void SidebarWidget::updateStats(int trainCount, int remainingSeats, int soldSeats)
{
    m_statsLabel->setText(QString("班次 %1  ·  余票 %2  ·  已售 %3")
                          .arg(trainCount).arg(remainingSeats).arg(soldSeats));
}