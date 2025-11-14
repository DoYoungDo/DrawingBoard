#include "drawer.h"


class DefaultPen : public Pen
{
    ~DefaultPen(){}
public:
    QPixmap shape() override{
        return QPixmap(":/res/Icon.png");
    }
};

Drawer::Drawer(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent,Qt::FramelessWindowHint | f}
{
    pensContainer << new DefaultPen();
}

Drawer::~Drawer()
{
    // qDeleteAll(pensContainer);
}

Pen* Drawer::currentPen()
{
    pensContainer.first();
}
