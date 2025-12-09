#include "capabilitybutton.h"

#include <QMouseEvent>
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

    if(this->isChecked())
    {
        p.save();
        p.setPen(QPen(Qt::gray, 2));
        p.drawLine(this->rect().bottomLeft(), this->rect().bottomRight());
        p.restore();
    }

    if(!this->isEnabled())
    {
        p.save();
        p.setPen(QPen(Qt::gray, 2));
        p.drawLine(this->rect().topLeft(), this->rect().bottomRight());
        p.restore();
    }
}
void CapabilityButton::mousePressEvent(QMouseEvent* event)
{
    QPushButton::mousePressEvent(event);
    event->accept();
}

void CapabilityButton::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

void CapabilityButton::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);
    event->accept();
}

void CapabilityButton::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPushButton::mouseDoubleClickEvent(event);
    event->accept();
}
