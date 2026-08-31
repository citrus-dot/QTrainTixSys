#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addtraindialog.h"
#include "ticketdialog.h"
#include "ticketview.h"
#include "querydialog.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->trainTable->setColumnCount(8);
    ui->trainTable->setHorizontalHeaderLabels({"班次号", "发车日期", "发车时间", "发车城市", "终点城市", "车厢数", "每厢座位数", "余票"});
    ui->trainTable->horizontalHeader()->setStretchLastSection(true);
    ui->trainTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->seatTable->setColumnCount(4);
    ui->seatTable->setHorizontalHeaderLabels({"车厢号", "座位号", "姓名", "身份证号"});
    ui->seatTable->horizontalHeader()->setStretchLastSection(true);
    ui->seatTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionAddTrain, &QAction::triggered, this, &MainWindow::onAddTrain);
    connect(ui->actionRemoveTrain, &QAction::triggered, this, &MainWindow::onRemoveTrain);
    connect(ui->actionSell, &QAction::triggered, this, &MainWindow::onSellTicket);
    connect(ui->actionRefund, &QAction::triggered, this, &MainWindow::onRefundTicket);
    connect(ui->actionQuery, &QAction::triggered, this, &MainWindow::onQuery);
    connect(ui->trainTable, &QTableWidget::cellClicked, this, &MainWindow::onTrainSelected);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewFile()
{
    m_system = TrainSystem();
    m_filePath.clear();
    refreshTrainTable();
    refreshSeatTable();
    setWindowTitle("列车客运售票管理系统");
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
    refreshTrainTable();
    refreshSeatTable();
    setWindowTitle(QString("列车客运售票管理系统 - %1").arg(path));
    ui->statusbar->showMessage(QString("已打开: %1").arg(path));
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
    ui->statusbar->showMessage(QString("已保存: %1").arg(path));
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
    refreshTrainTable();
    ui->statusbar->showMessage(QString("已新增班次 %1").arg(t.no()));
}

void MainWindow::onRemoveTrain()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先选择一个班次");
        return;
    }
    if (QMessageBox::question(this, "确认", QString("确定删除班次 %1 吗？").arg(t->no())) != QMessageBox::Yes)
        return;
    m_system.removeTrain(t->no());
    refreshTrainTable();
    refreshSeatTable();
    ui->statusbar->showMessage(QString("已删除班次 %1").arg(t->no()));
}

void MainWindow::onSellTicket()
{
    Train *t = currentTrain();
    if (!t) {
        QMessageBox::information(this, "提示", "请先选择一个班次");
        return;
    }
    TicketDialog dlg(t, this);
    dlg.setSellMode(true);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (t->sellTicket(dlg.name(), dlg.id(), dlg.carriage(), dlg.seatNo())) {
        refreshSeatTable();
        refreshTrainTable();
        ui->statusbar->showMessage("售票成功");
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
        QMessageBox::information(this, "提示", "请先选择一个班次");
        return;
    }
    TicketDialog dlg(t, this);
    dlg.setSellMode(false);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (t->refundTicket(dlg.carriage(), dlg.seatNo())) {
        refreshSeatTable();
        refreshTrainTable();
        ui->statusbar->showMessage("退票成功");
    }
}

void MainWindow::onQuery()
{
    QueryDialog dlg(m_system, this);
    dlg.exec();
}

void MainWindow::onTrainSelected(int row, int column)
{
    Q_UNUSED(column);
    refreshSeatTable();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于", "列车客运售票管理系统\n课程设计作业");
}

void MainWindow::onSearchChanged(const QString &text)
{
    Q_UNUSED(text);
    refreshTrainTable();
    refreshSeatTable();
}

void MainWindow::refreshTrainTable()
{
    const QVector<Train> &trains = m_system.trains();
    const QString keyword = ui->searchEdit->text().trimmed();
    ui->trainTable->setRowCount(0);
    int row = 0;
    for (const Train &t : trains) {
        if (!keyword.isEmpty()
            && !t.no().contains(keyword, Qt::CaseInsensitive)
            && !t.from().contains(keyword, Qt::CaseInsensitive)
            && !t.to().contains(keyword, Qt::CaseInsensitive))
            continue;
        QTableWidgetItem *noItem = new QTableWidgetItem(t.no());
        noItem->setData(Qt::UserRole, t.no());
        ui->trainTable->insertRow(row);
        ui->trainTable->setItem(row, 0, noItem);
        ui->trainTable->setItem(row, 1, new QTableWidgetItem(t.date()));
        ui->trainTable->setItem(row, 2, new QTableWidgetItem(t.departTime()));
        ui->trainTable->setItem(row, 3, new QTableWidgetItem(t.from()));
        ui->trainTable->setItem(row, 4, new QTableWidgetItem(t.to()));
        ui->trainTable->setItem(row, 5, new QTableWidgetItem(QString::number(t.carriages())));
        ui->trainTable->setItem(row, 6, new QTableWidgetItem(QString::number(t.seatsPerCarriage())));
        ui->trainTable->setItem(row, 7, new QTableWidgetItem(QString::number(t.remainingSeats())));
        ++row;
    }
}

void MainWindow::refreshSeatTable()
{
    ui->seatTable->setRowCount(0);
    Train *t = currentTrain();
    if (!t)
        return;
    const QVector<Seat> &seats = t->seats();
    ui->seatTable->setRowCount(seats.size());
    for (int i = 0; i < seats.size(); ++i) {
        const Seat &s = seats[i];
        ui->seatTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.carriage())));
        ui->seatTable->setItem(i, 1, new QTableWidgetItem(QString::number(s.seatNo())));
        ui->seatTable->setItem(i, 2, new QTableWidgetItem(s.isEmpty() ? "空闲" : s.name()));
        ui->seatTable->setItem(i, 3, new QTableWidgetItem(s.isEmpty() ? "-" : s.id()));
    }
}

Train *MainWindow::currentTrain()
{
    int row = ui->trainTable->currentRow();
    if (row < 0)
        return nullptr;
    QTableWidgetItem *item = ui->trainTable->item(row, 0);
    if (!item)
        return nullptr;
    return m_system.findTrain(item->data(Qt::UserRole).toString());
}
