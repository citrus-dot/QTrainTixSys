#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addtraindialog.h"
#include "ticketdialog.h"
#include "ticketview.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 顶部栏：文件操作
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveFile);

    // 操作按钮：新增/删除/售票/退票
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::onAddTrain);
    connect(ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveTrain);
    connect(ui->sellButton, &QPushButton::clicked, this, &MainWindow::onSellTicket);
    connect(ui->refundButton, &QPushButton::clicked, this, &MainWindow::onRefundTicket);

    // 菜单
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // 侧边栏导航
    connect(ui->sidebar, &SidebarWidget::pageSelected, this, &MainWindow::onPageSelected);
    connect(ui->sidebar, &SidebarWidget::aboutClicked, this, &MainWindow::onAbout);

    // 班次列表：选中刷新座位表，双击直接售票
    connect(ui->trainListView, &TrainListView::trainSelected, this, &MainWindow::refreshSeatTable);
    connect(ui->trainListView, &TrainListView::trainDoubleClicked, this, &MainWindow::onSellTicket);

    // 查询页绑定数据源
    ui->queryPageWidget->setSystem(&m_system);

    // 按钮图标与主操作强调
    ui->openButton->setIcon(QIcon(":/icons/file-open.svg"));
    ui->saveButton->setIcon(QIcon(":/icons/file-save.svg"));
    ui->addButton->setIcon(QIcon(":/icons/action-add.svg"));
    ui->removeButton->setIcon(QIcon(":/icons/action-delete.svg"));
    ui->sellButton->setIcon(QIcon(":/icons/action-ticket.svg"));
    ui->refundButton->setIcon(QIcon(":/icons/action-refund.svg"));
    ui->sellButton->setProperty("primary", true);

    // 状态栏常驻统计
    m_statsLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(m_statsLabel);

    // 默认进入班次管理页
    ui->sidebar->setCurrentPage(0);
    onPageSelected(0);
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
    refreshSeatTable();
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
    refreshSeatTable();
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
    refreshSeatTable();
    ui->statusbar->showMessage(QString("已删除班次 %1").arg(t->no()), 3000);
}

void MainWindow::onSellTicket()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    TicketDialog dlg(t, this);
    dlg.setSellMode(true);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (t->sellTicket(dlg.name(), dlg.id(), dlg.carriage(), dlg.seatNo())) {
        refreshSeatTable();
        refreshTrainList();
        ui->statusbar->showMessage("售票成功", 3000);
        TicketView view(t, dlg.name(), dlg.id(), dlg.carriage(), dlg.seatNo(), this);
        view.exec();
    } else {
        QMessageBox::warning(this, "提示", "售票失败");
    }
}

void MainWindow::onRefundTicket()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先在左侧选择一个班次");
        return;
    }
    TicketDialog dlg(t, this);
    dlg.setSellMode(false);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (t->refundTicket(dlg.carriage(), dlg.seatNo())) {
        refreshSeatTable();
        refreshTrainList();
        ui->statusbar->showMessage("退票成功", 3000);
    }
}

void MainWindow::onQuery()
{
    onPageSelected(1); // 查询已整合为独立页面，直接切换到查询页
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于",
                       "<h3>列车客运售票管理系统</h3>"
                       "<p>课程设计作业 · Qt Widgets</p>"
                       "<p>支持班次管理、售票退票、余票查询、车票导出</p>");
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

void MainWindow::refreshSeatTable()
{
    ui->seatTableView->setTrain(currentTrain());
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
