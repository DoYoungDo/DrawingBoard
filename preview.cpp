#include "preview.h"

#include <QPainter>


Preview::Preview(const QPixmap& pix, QWidget* parent)
    : QWidget{parent}
{
    this->pix = pix;
}

void Preview::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);
    p.drawPixmap(this->rect(), pix);
}
