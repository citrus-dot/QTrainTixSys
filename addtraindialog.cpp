#include "addtraindialog.h"
#include "ui_addtraindialog.h"
#include <QMessageBox>
#include <QDate>
#include <QCalendarWidget>
#include <QDialog>
#include <QIcon>
#include <QSet>
#include <QSize>
#include <QPoint>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

AddTrainDialog::AddTrainDialog(QWidget *parent)
    : FadeDialog(parent)
    , ui(new Ui::AddTrainDialog)
{
    ui->setupUi(this);
    m_selectedDate = QDate::currentDate();
    ui->dateEdit->setText(m_selectedDate.toString("yyyy-MM-dd"));
    ui->dateBtn->setIconSize(QSize(14, 14));
    updateCalendarIcon();
    connect(ui->dateBtn, &QToolButton::clicked, this, &AddTrainDialog::toggleCalendar);
}

// 编辑模式：把已有班次的字段回填到表单
AddTrainDialog::AddTrainDialog(const Train &editTrain, QWidget *parent)
    : AddTrainDialog(parent)
{
    ui->titleLabel->setText("修改班次");
    setWindowTitle("修改班次");
    ui->noEdit->setText(editTrain.no());
    ui->timeEdit->setText(editTrain.departTime());
    ui->fromEdit->setText(editTrain.from());
    ui->toEdit->setText(editTrain.to());
    ui->carriagesSpin->setValue(editTrain.carriages());
    ui->seatsSpin->setValue(editTrain.seatsPerCarriage());
    if (!editTrain.date().isEmpty()) {
        m_selectedDate = QDate::fromString(editTrain.date(), "yyyy-MM-dd");
        ui->dateEdit->setText(editTrain.date());
    }
    ui->firstPriceSpin->setValue(editTrain.firstClassPrice());
    ui->secondPriceSpin->setValue(editTrain.secondClassPrice());
    // 回填一等车厢号（任意布局，如 D2206 的"2"）
    QStringList firstCars;
    for (int c = 1; c <= editTrain.carriages(); ++c)
        if (editTrain.carriageClass(c) == 1)
            firstCars << QString::number(c);
    ui->classEdit->setText(firstCars.join(","));
    ui->stopsEdit->setText(editTrain.stops().join(","));
}

AddTrainDialog::~AddTrainDialog()
{
    delete ui;
}

void AddTrainDialog::toggleCalendar()
{
    if (m_calendarVisible)
        closeCalendar();
    else
        openCalendar();
}

void AddTrainDialog::openCalendar()
{
    if (!m_calendarPopup) {
        m_calendarPopup = new QDialog(this);
        m_calendarPopup->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
        auto *cal = new QCalendarWidget(m_calendarPopup);
        cal->setSelectedDate(m_selectedDate);
        auto *layout = new QVBoxLayout(m_calendarPopup);
        layout->setContentsMargins(1, 1, 1, 1);
        layout->addWidget(cal);
        connect(cal, &QCalendarWidget::clicked, this, [this](const QDate &date) {
            m_selectedDate = date;
            ui->dateEdit->setText(date.toString("yyyy-MM-dd"));
            closeCalendar();
        });
    }
    m_calendarPopup->move(ui->dateField->mapToGlobal(QPoint(0, ui->dateField->height())));
    m_calendarPopup->show();
    m_calendarPopup->raise();
    m_calendarPopup->activateWindow();
    m_calendarVisible = true;
    updateCalendarIcon();
    qApp->installEventFilter(this);
}

void AddTrainDialog::closeCalendar()
{
    if (m_calendarPopup)
        m_calendarPopup->hide();
    m_calendarVisible = false;
    updateCalendarIcon();
    qApp->removeEventFilter(this);
}

void AddTrainDialog::updateCalendarIcon()
{
    ui->dateBtn->setIcon(QIcon(m_calendarVisible ? ":/icons/chevron-up.png" : ":/icons/chevron-down.png"));
}

bool AddTrainDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (m_calendarVisible && event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        const QPoint global = me->globalPosition().toPoint();
        const bool onArrow = ui->dateBtn->rect().contains(ui->dateBtn->mapFromGlobal(global));
        if (!m_calendarPopup->geometry().contains(global) && !onArrow)
            closeCalendar();
    }
    return FadeDialog::eventFilter(obj, event);
}

Train AddTrainDialog::getTrain() const
{
    Train t(ui->noEdit->text().trimmed(),
            ui->dateEdit->text(),
            ui->timeEdit->text().trimmed(),
            ui->fromEdit->text().trimmed(),
            ui->toEdit->text().trimmed(),
            ui->carriagesSpin->value(),
            ui->seatsSpin->value(),
            ui->firstPriceSpin->value(),
            ui->secondPriceSpin->value());
    // 一等车厢号列表（如 "1" 或 "2,3"），未列出的车厢均为二等
    QVector<int> classes(t.carriages(), 2);
    const QStringList firstCars = ui->classEdit->text().split(',', Qt::SkipEmptyParts);
    for (const QString &s : firstCars) {
        const int c = s.trimmed().toInt();
        if (c >= 1 && c <= t.carriages())
            classes[c - 1] = 1;
    }
    t.setCarriageClass(classes);
    QStringList stops;
    for (const QString &s : ui->stopsEdit->text().split(',', Qt::SkipEmptyParts))
        stops << s.trimmed();
    t.setStops(stops);
    return t;
}

void AddTrainDialog::accept()
{
    if (ui->noEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "班次号不能为空");
        return;
    }
    if (ui->fromEdit->text().trimmed().isEmpty() || ui->toEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "发车城市和终点城市不能为空");
        return;
    }
    if (!Train::isValidTime(ui->timeEdit->text().trimmed())) {
        QMessageBox::warning(this, "提示", "发车时间格式应为 HH:MM");
        return;
    }
    // 一等车厢号校验：必须是 1..车厢数 内的整数，且不重复
    QStringList firstCars = ui->classEdit->text().split(',', Qt::SkipEmptyParts);
    QSet<int> seen;
    for (const QString &s : firstCars) {
        bool ok = false;
        const int c = s.trimmed().toInt(&ok);
        if (!ok || c < 1 || c > ui->carriagesSpin->value()) {
            QMessageBox::warning(this, "提示",
                                 QString("一等车厢号 %1 无效，应在 1~%2 之间")
                                     .arg(s.trimmed()).arg(ui->carriagesSpin->value()));
            return;
        }
        if (seen.contains(c)) {
            QMessageBox::warning(this, "提示", QString("一等车厢号 %1 重复").arg(c));
            return;
        }
        seen.insert(c);
    }
    if (ui->firstPriceSpin->value() <= 0 || ui->secondPriceSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "票价必须大于 0");
        return;
    }
    QDialog::accept();
}
