#ifndef PEN_H
#define PEN_H

#include <QPen>

class Pen : public QPen
{
public:
    Pen():QPen(){}
    Pen(Qt::PenStyle s):QPen(s){}
    Pen(const QColor &color):QPen(color){}
    Pen(const QBrush &brush, qreal width, Qt::PenStyle s = Qt::SolidLine,Qt::PenCapStyle c = Qt::SquareCap, Qt::PenJoinStyle j = Qt::BevelJoin)
        :QPen(brush, width, s, c ,j){}
    Pen(const QPen &pen) noexcept:QPen(pen){}

    virtual ~Pen() = default;
    virtual QPixmap shape() = 0;
};

#endif // PEN_H
