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
    // 自定义 QWidget 子类需启用该属性，QSS 的 background 才能生效
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(220);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 20, 14, 14);
    layout->setSpacing(6);

    // 品牌区：图标 + 标题 + 副标题
    auto *brandRow = new QHBoxLayout;
    brandRow->setSpacing(10);
    auto *brandIcon = new QLabel(this);
    brandIcon->setPixmap(QIcon(":/icons/nav-train.svg").pixmap(28, 28));
    auto *brandText = new QLabel("列车售票系统", this);
    brandText->setObjectName("sidebarBrand");
    brandRow->addWidget(brandIcon);
    brandRow->addWidget(brandText);
    brandRow->addStretch();
    layout->addLayout(brandRow);

    auto *subtitle = new QLabel("客运售票管理系统", this);
    subtitle->setObjectName("sidebarSubtitle");
    layout->addWidget(subtitle);

    layout->addSpacing(14);

    // 导航分组标题
    auto *navLabel = new QLabel("功能导航", this);
    navLabel->setObjectName("sidebarSection");
    layout->addWidget(navLabel);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    auto addNav = [&](const QString &text, const QString &icon, int id) {
        auto *btn = new QPushButton(QIcon(icon), text, this);
        btn->setProperty("nav", true);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIconSize(QSize(20, 20));
        m_group->addButton(btn, id);
        m_navButtons.append(btn);
        layout->addWidget(btn);
    };

    addNav("班次管理", ":/icons/nav-train.svg", 0);
    addNav("查询", ":/icons/nav-search.svg", 1);

    layout->addStretch();

    // 底部信息卡：填充空白并传达数据存储方式
    auto *infoCard = new QFrame(this);
    infoCard->setObjectName("sidebarInfo");
    auto *infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(12, 10, 12, 10);
    infoLayout->setSpacing(4);
    auto *infoTitle = new QLabel("本地数据存储", infoCard);
    infoTitle->setObjectName("sidebarInfoTitle");
    auto *infoSub = new QLabel("数据保存在 .dat 文件", infoCard);
    infoSub->setObjectName("sidebarInfoSub");
    infoLayout->addWidget(infoTitle);
    infoLayout->addWidget(infoSub);
    layout->addWidget(infoCard);

    layout->addSpacing(6);

    auto *aboutBtn = new QPushButton(QIcon(":/icons/about.svg"), "关于", this);
    aboutBtn->setProperty("nav", true);
    aboutBtn->setCursor(Qt::PointingHandCursor);
    aboutBtn->setIconSize(QSize(20, 20));
    layout->addWidget(aboutBtn);
    connect(aboutBtn, &QPushButton::clicked, this, &SidebarWidget::aboutClicked);

    auto *version = new QLabel("版本 V16", this);
    version->setObjectName("sidebarVersion");
    version->setAlignment(Qt::AlignCenter);
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
