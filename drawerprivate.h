#ifndef DRAWERPRIVATE_H
#define DRAWERPRIVATE_H

#include <QColor>

class QSlider;

class DrawerPrivate{
    DrawerPrivate();

    void addShowOrHide(std::function<void(std::function<void()> oldCall, bool v)> showOrHide);

    friend class Drawer;

    QColor backgroundColor;

    std::function<void()> collapse = nullptr;
    std::function<void()> expand = nullptr;
    bool isExpand = true;
    QByteArray lastGeometry;
};


#endif // DRAWERPRIVATE_H
