#include "registerdialog.h"
#include "train.h"
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent)
    : FadeDialog(parent)
{
    setWindowTitle("登记旅客");
    setMinimumWidth(320);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(14);

    auto *title = new QLabel("登记旅客", this);
    title->setObjectName("panelTitle");
    layout->addWidget(title);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(10);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("请输入旅客姓名");
    m_idEdit = new QLineEdit(this);
    m_idEdit->setPlaceholderText("18 位身份证号");
    form->addRow("姓名", m_nameEdit);
    form->addRow("身份证号", m_idEdit);
    layout->addLayout(form);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    box->button(QDialogButtonBox::Ok)->setText("登记");
    box->button(QDialogButtonBox::Cancel)->setText("取消");
    connect(box, &QDialogButtonBox::accepted, this, &RegisterDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &RegisterDialog::reject);
    layout->addWidget(box);
}

QString RegisterDialog::name() const { return m_nameEdit->text().trimmed(); }
QString RegisterDialog::id() const { return m_idEdit->text().trimmed(); }

void RegisterDialog::accept()
{
    if (name().isEmpty()) {
        QMessageBox::warning(this, "提示", "姓名不能为空");
        return;
    }
    if (!Train::isValidId(id())) {
        QMessageBox::warning(this, "提示", "身份证号格式不正确（18位）");
        return;
    }
    QDialog::accept();
}
