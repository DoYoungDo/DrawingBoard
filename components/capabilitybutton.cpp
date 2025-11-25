#include "capabilitybutton.h"

#include <QPainter>

CapabilityButton::CapabilityButton(const QIcon& ic, QWidget* parent)
    :QPushButton(ic,QString(),parent)
{}

void CapabilityButton::paintEvent(QPaintEvent* event)
{
    QIcon ic = this->icon();
    if(ic.isNull())
    {
        QPushButton::paintEvent(event);
        return;
    }

    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);
    p.drawPixmap(QRect(this->rect().topLeft(), this->iconSize()), ic.pixmap(this->iconSize(), (this->isCheckable() && this->isChecked()) ? QIcon::Selected : QIcon::Normal));
}
