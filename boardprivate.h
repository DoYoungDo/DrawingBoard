#ifndef BOARDPRIVATE_H
#define BOARDPRIVATE_H

#include <QStack>


class QImage;
class QPainter;

class Board;
class Drawer;

class BoardPrivate{
public:
    enum State{
        READY_TO_DRAW = 0,
        NOT_READY_TO_DRAW,
        DISPLAY
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

    bool showOrHideDrawer(QPoint p);


    friend class Board;
    Board* q = nullptr;

    QImage backgroundImg;
    QImage boradImg;
    QImage foregroundImg;

    State state;
    QStack<State> stateStack;

    bool mouseIsPress = false;

    Drawer* controlPlatform = nullptr;
};

#endif // BOARDPRIVATE_H
