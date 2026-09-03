#ifndef FADEDIALOG_H
#define FADEDIALOG_H

#include <QDialog>

// 对话框基类：显示时淡入，提供自然的过渡反馈
class FadeDialog : public QDialog
{
    Q_OBJECT
public:
    using QDialog::QDialog;

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // FADEDIALOG_H