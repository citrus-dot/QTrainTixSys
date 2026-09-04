#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "fadedialog.h"

class QLineEdit;

// 手动登记旅客对话框：输入姓名和身份证号
class RegisterDialog : public FadeDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr);

    QString name() const;
    QString id() const;

protected:
    void accept() override;

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_idEdit;
};

#endif // REGISTERDIALOG_H
