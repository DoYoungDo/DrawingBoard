#ifndef BOARDPRIVATE_H
#define BOARDPRIVATE_H

#include <QImage>
#include <QPixmap>
#include <QStack>

class QPainter;
class QPoint;

class Board;
class Drawer;
class Preview;

class BoardPrivate{
public:
    enum State{
        NONE = 0,
        SHOW_BACKGROUND = 1 << 0,
        SHOW_FOREGTOUND = 1 << 1,
        SHOW_CONTROL= 1 << 2,

        READY_TO_DRAW = SHOW_BACKGROUND | SHOW_FOREGTOUND | SHOW_CONTROL,

        CONTROL_VISIBLE_SWITCHING = 1 << 3,

        INIT = READY_TO_DRAW
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

    bool tryShowDrawer();
    bool tryHideDrawer();
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
    QRect savedControlPlatformGeometry;

    Preview* previewPort = nullptr;
};

#endif // BOARDPRIVATE_H
