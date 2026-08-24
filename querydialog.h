#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QDialog>
#include "trainsystem.h"

namespace Ui { class QueryDialog; }

class QueryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryDialog(const TrainSystem &system, QWidget *parent = nullptr);
    ~QueryDialog() override;

private slots:
    void onTrainChanged();

private:
    Ui::QueryDialog *ui;
    const TrainSystem &m_system;
};

#endif // QUERYDIALOG_H
