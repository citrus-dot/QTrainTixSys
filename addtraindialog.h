#ifndef ADDTRAINDIALOG_H
#define ADDTRAINDIALOG_H

#include <QDialog>
#include "train.h"

namespace Ui { class AddTrainDialog; }

class AddTrainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTrainDialog(QWidget *parent = nullptr);
    ~AddTrainDialog() override;

    Train getTrain() const;

protected:
    void accept() override;

private:
    Ui::AddTrainDialog *ui;
};

#endif // ADDTRAINDIALOG_H
