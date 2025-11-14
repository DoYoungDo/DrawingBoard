#ifndef PEN_H
#define PEN_H

#include <QPen>

class Pen : public QPen
{
public:
    virtual QPixmap shape() = 0;
};

#endif // PEN_H
