#include "seattabledialog.h"
#include "seattableview.h"
#include <QVBoxLayout>

SeatTableDialog::SeatTableDialog(QWidget *parent)
    : FadeDialog(parent)
{
    setWindowTitle("座位登记");
    setMinimumSize(620, 480);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(0);

    m_view = new SeatTableView(this);
    layout->addWidget(m_view);

    connect(m_view, &SeatTableView::dataChanged, this, &SeatTableDialog::dataChanged);
}

void SeatTableDialog::setTrain(Train *train)
{
    m_view->setTrain(train);
}
