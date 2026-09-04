#ifndef ADDTRAINDIALOG_H
#define ADDTRAINDIALOG_H

#include "fadedialog.h"
#include "train.h"
#include <QDate>

class QDialog;

namespace Ui { class AddTrainDialog; }

class AddTrainDialog : public FadeDialog
{
    Q_OBJECT

public:
    explicit AddTrainDialog(QWidget *parent = nullptr);
    ~AddTrainDialog() override;

    Train getTrain() const;

protected:
    void accept() override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void toggleCalendar();

private:
    void openCalendar();
    void closeCalendar();
    void updateCalendarIcon();

    Ui::AddTrainDialog *ui;
    QDate m_selectedDate;
    QDialog *m_calendarPopup = nullptr;
    bool m_calendarVisible = false;
};

#endif // ADDTRAINDIALOG_H
