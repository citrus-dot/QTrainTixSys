#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->trainTable->setColumnCount(7);
    ui->trainTable->setHorizontalHeaderLabels({"班次号", "发车时间", "发车城市", "终点城市", "车厢数", "每厢座位数", "余票"});
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
    connect(ui->trainTable, &QTableWidget::cellClicked, this, &MainWindow::onTrainSelected);
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

void MainWindow::onTrainSelected(int row, int column)
{
    Q_UNUSED(column);
    refreshSeatTable();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于", "列车客运售票管理系统\n课程设计作业");
}

void MainWindow::refreshTrainTable()
{
    const QVector<Train> &trains = m_system.trains();
    ui->trainTable->setRowCount(trains.size());
    for (int i = 0; i < trains.size(); ++i) {
        const Train &t = trains[i];
        ui->trainTable->setItem(i, 0, new QTableWidgetItem(t.no()));
        ui->trainTable->setItem(i, 1, new QTableWidgetItem(t.departTime()));
        ui->trainTable->setItem(i, 2, new QTableWidgetItem(t.from()));
        ui->trainTable->setItem(i, 3, new QTableWidgetItem(t.to()));
        ui->trainTable->setItem(i, 4, new QTableWidgetItem(QString::number(t.carriages())));
        ui->trainTable->setItem(i, 5, new QTableWidgetItem(QString::number(t.seatsPerCarriage())));
        ui->trainTable->setItem(i, 6, new QTableWidgetItem(QString::number(t.remainingSeats())));
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

Train *MainWindow::currentTrain() const
{
    int row = ui->trainTable->currentRow();
    if (row < 0 || row >= m_system.trains().size())
        return nullptr;
    return const_cast<Train *>(&m_system.trains()[row]);
}
