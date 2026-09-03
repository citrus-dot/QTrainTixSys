#include "fadedialog.h"
#include <QPropertyAnimation>
#include <QShowEvent>

void FadeDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    auto *anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(200);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}