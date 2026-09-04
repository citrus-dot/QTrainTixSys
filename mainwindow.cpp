#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addtraindialog.h"
#include "ticketdialog.h"
#include "ticketview.h"
#include "selldialog.h"
#include "refunddialog.h"
#include "seattabledialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QEvent>

namespace {
// QSS 的 qproperty-icon 在 :hover 下不可靠，改用事件过滤器切换图标
class HoverIconFilter : public QObject
{
public:
    HoverIconFilter(QPushButton *btn, const QString &normal, const QString &hover, QObject *parent)
        : QObject(parent), m_btn(btn), m_normal(normal), m_hover(hover)
    {
        m_btn->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override
    {
        if (obj == m_btn) {
            if (ev->type() == QEvent::Enter)
                m_btn->setIcon(QIcon(m_hover));
            else if (ev->type() == QEvent::Leave)
                m_btn->setIcon(QIcon(m_normal));
        }
        return QObject::eventFilter(obj, ev);
    }

private:
    QPushButton *m_btn;
    QString m_normal;
    QString m_hover;
};

void setupHoverIcon(QPushButton *btn, const QString &normal, const QString &hover, QObject *parent)
{
    btn->setIcon(QIcon(normal));
    new HoverIconFilter(btn, normal, hover, parent);
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 顶部栏：文件操作
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveFile);

    // 操作按钮：新增/删除/售票/退票/座位登记
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddTrain);
    connect(ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveTrain);
    connect(ui->sellButton, &QPushButton::clicked, this, &MainWindow::onSellTicket);
    connect(ui->refundButton, &QPushButton::clicked, this, &MainWindow::onRefundTicket);
    connect(ui->seatButton, &QPushButton::clicked, this, &MainWindow::onSeatTable);

    // 菜单
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // 侧边栏导航
    connect(ui->sidebar, &SidebarWidget::pageSelected, this, &MainWindow::onPageSelected);
    connect(ui->sidebar, &SidebarWidget::aboutClicked, this, &MainWindow::onAbout);

    // 班次列表：选中变化联动按钮状态，双击打开班次详情
    connect(ui->trainListView, &TrainListView::trainSelected, this, &MainWindow::onTrainSelectionChanged);
    connect(ui->trainListView, &TrainListView::trainDoubleClicked, this, &MainWindow::onTrainDoubleClicked);

    // 查询页绑定数据源
    ui->queryPageWidget->setSystem(&m_system);

    // 按钮图标：默认灰色，悬停变白（由事件过滤器切换）
    setupHoverIcon(ui->openButton, ":/icons/file-open.svg", ":/icons/file-open-white.svg", this);
    setupHoverIcon(ui->saveButton, ":/icons/file-save.svg", ":/icons/file-save-white.svg", this);
    setupHoverIcon(ui->addButton, ":/icons/action-add.svg", ":/icons/action-add-white.svg", this);
    setupHoverIcon(ui->removeButton, ":/icons/action-delete.svg", ":/icons/action-delete-white.svg", this);
    setupHoverIcon(ui->sellButton, ":/icons/action-ticket.svg", ":/icons/action-ticket-white.svg", this);
    setupHoverIcon(ui->refundButton, ":/icons/action-refund.svg", ":/icons/action-refund-white.svg", this);
    setupHoverIcon(ui->seatButton, ":/icons/action-seat.svg", ":/icons/action-seat-white.svg", this);

    // 状态栏常驻统计
    m_statsLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(m_statsLabel);

    // 默认进入班次管理页
    ui->sidebar->setCurrentPage(0);
    onPageSelected(0);

    // 初始刷新：触发班次列表空状态引导与查询页初始化
    refreshTrainList();
    // 初始无选中班次，禁用依赖选中行的操作按钮
    onTrainSelectionChanged(QString());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewFile()
{
    m_system = TrainSystem();
    m_filePath.clear();
    ui->trainListView->clearSearch();
    refreshTrainList();
    setWindowTitle("列车客运售票管理系统");
    ui->statusbar->showMessage("已新建空数据", 3000);
}

void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(this, "打开数据文件", QString(), "数据文件 (*.dat)");
    if (path.isEmpty())
        return;
    if (!m_system.loadFromFile(path)) {
        QMessageBox::warning(this, "错误", "文件打开失败");
        return;
    }
    m_filePath = path;
    ui->trainListView->clearSearch();
    refreshTrainList();
    setWindowTitle(QString("列车客运售票管理系统 - %1").arg(path));
    ui->statusbar->showMessage(QString("已打开: %1").arg(path), 3000);
}

void MainWindow::onSaveFile()
{
    QString path = m_filePath;
    if (path.isEmpty())
        path = QFileDialog::getSaveFileName(this, "保存数据文件", QString(), "数据文件 (*.dat)");
    if (path.isEmpty())
        return;
    if (!m_system.saveToFile(path)) {
        QMessageBox::warning(this, "错误", "文件保存失败");
        return;
    }
    m_filePath = path;
    setWindowTitle(QString("列车客运售票管理系统 - %1").arg(path));
    ui->statusbar->showMessage(QString("已保存: %1").arg(path), 3000);
}

void MainWindow::onAddTrain()
{
    AddTrainDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    Train t = dlg.getTrain();
    if (!m_system.addTrain(t)) {
        QMessageBox::warning(this, "提示", "班次号已存在");
        return;
    }
    refreshTrainList();
    ui->statusbar->showMessage(QString("已新增班次 %1").arg(t.no()), 3000);
}

void MainWindow::onRemoveTrain()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    if (QMessageBox::question(this, "确认删除",
                              QString("确定删除班次 %1（%2 → %3）吗？")
                                  .arg(t->no(), t->from(), t->to()))
        != QMessageBox::Yes)
        return;
    m_system.removeTrain(t->no());
    refreshTrainList();
    ui->statusbar->showMessage(QString("已删除班次 %1").arg(t->no()), 3000);
}

void MainWindow::onSellTicket()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    SellDialog dlg(this);
    dlg.setTrain(t);
    connect(&dlg, &SellDialog::dataChanged, this, &MainWindow::refreshTrainList);
    dlg.exec();
}

void MainWindow::onRefundTicket()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    RefundDialog dlg(this);
    dlg.setTrain(t);
    connect(&dlg, &RefundDialog::dataChanged, this, &MainWindow::refreshTrainList);
    dlg.exec();
}

void MainWindow::onTrainDoubleClicked()
{
    Train *t = currentTrain();
    if (!t)
        return;
    TicketDialog dlg(t, this);
    connect(&dlg, &TicketDialog::dataChanged, this, &MainWindow::refreshTrainList);
    dlg.exec();
}

void MainWindow::onQuery()
{
    onPageSelected(1); // 查询已整合为独立页面，直接切换到查询页
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于",
                       "<h3>列车客运售票管理系统</h3>"
                       "<p>版本 v0.1.16</p>"
                       "<p>课程设计作业 · Qt Widgets</p>"
                       "<p>支持班次管理、售票退票、余票查询、车票导出</p>");
}

void MainWindow::onSeatTable()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    SeatTableDialog dlg(this);
    dlg.setTrain(t);
    connect(&dlg, &SeatTableDialog::dataChanged, this, &MainWindow::refreshTrainList);
    dlg.exec();
}

void MainWindow::onTrainSelectionChanged(const QString &no)
{
    const bool has = !no.isEmpty();
    ui->removeButton->setEnabled(has);
    ui->sellButton->setEnabled(has);
    ui->refundButton->setEnabled(has);
    ui->seatButton->setEnabled(has);
}

void MainWindow::onPageSelected(int index)
{
    ui->pages->setCurrentIndex(index);
    ui->pageTitle->setText(index == 0 ? "班次管理" : "查询");
}

void MainWindow::refreshTrainList()
{
    ui->trainListView->setTrains(m_system.trains());
    ui->queryPageWidget->refresh();
    updateStats();
}

void MainWindow::updateStats()
{
    int remaining = 0, capacity = 0;
    for (const Train &t : m_system.trains()) {
        remaining += t.remainingSeats();
        capacity += t.carriages() * t.seatsPerCarriage();
    }
    m_statsLabel->setText(QString("班次 %1    余票 %2    已售 %3")
                          .arg(m_system.trains().size())
                          .arg(remaining)
                          .arg(capacity - remaining));
}

Train *MainWindow::currentTrain()
{
    return m_system.findTrain(ui->trainListView->currentTrainNo());
}
