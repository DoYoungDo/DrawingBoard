#ifndef DRAWER_H
#define DRAWER_H

#include "pen.h"

#include <QWidget>

class Drawer : public QWidget
{
    Q_OBJECT
public:
    explicit Drawer(QWidget *parent = nullptr,  Qt::WindowFlags f = Qt::WindowFlags());
    ~Drawer();

    Pen* currentPen();

private:
    QList<Pen*> pensContainer;
};

#endif // DRAWER_H
