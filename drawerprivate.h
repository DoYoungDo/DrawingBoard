#ifndef DRAWERPRIVATE_H
#define DRAWERPRIVATE_H

#include <QColor>

class QSlider;

class DrawerPrivate{
    DrawerPrivate();

    friend class Drawer;

    QSlider* backgroundAlphaSlider;
    QSlider* penSizeSlider;

    QColor backgroundColor;
};


#endif // DRAWERPRIVATE_H
