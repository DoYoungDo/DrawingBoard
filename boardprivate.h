#ifndef BOARDPRIVATE_H
#define BOARDPRIVATE_H

#include <QImage>
#include <QPixmap>
#include <QStack>

class QPainter;
class QPoint;

class Board;
class Drawer;

class BoardPrivate{
public:
    enum State{
        NONE = 0,
        SHOW_BACKGROUND = 1 << 0,
        SHOW_FOREGTOUND = 1 << 1,

        READY_TO_DRAW = SHOW_BACKGROUND | SHOW_FOREGTOUND,

        SHOW_CONTROL= 1 << 2,

        INIT = READY_TO_DRAW | SHOW_CONTROL
    };
private:
    BoardPrivate(Board* _q);
    ~BoardPrivate();

public:
    void drawBackgroundImg(QPainter* p);
    void drawBoradImg(QPainter* p);
    void drawForeGroundImg(QPainter* p);

    void savaState();
    void restoreState();
    void setState(State s);

    bool showOrHideDrawer(QPoint p);


    friend class Board;
    Board* q = nullptr;

    QPixmap backgroundCanvas;
    QPixmap boradCanvas;
    QPixmap foregroundCanvas;

    State state;
    QStack<State> stateStack;

    bool mouseIsPress = false;
    QPoint mouseLastPos;

    Drawer* controlPlatform = nullptr;
    QByteArray savedControlPlatformGeometry;
    QRect savedControlPlatformGeometry1;
};

#endif // BOARDPRIVATE_H
