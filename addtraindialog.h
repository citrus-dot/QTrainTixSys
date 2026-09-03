#ifndef ADDTRAINDIALOG_H
#define ADDTRAINDIALOG_H

#include "fadedialog.h"
#include "train.h"

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

private:
    Ui::AddTrainDialog *ui;
};

#endif // ADDTRAINDIALOG_H
