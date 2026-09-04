#include "addtraindialog.h"
#include "ui_addtraindialog.h"
#include <QMessageBox>
#include <QDate>
#include <QCalendarWidget>
#include <QDialog>
#include <QIcon>
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
    connect(ui->carriagesSpin, &QSpinBox::valueChanged, this, [this](int value) {
        ui->firstClassSpin->setMaximum(value);
    });
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
    QVector<int> classes;
    int firstCount = ui->firstClassSpin->value();
    for (int i = 0; i < t.carriages(); ++i)
        classes << (i < firstCount ? 1 : 2);
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
    if (ui->firstPriceSpin->value() <= 0 || ui->secondPriceSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "票价必须大于 0");
        return;
    }
    QDialog::accept();
}
