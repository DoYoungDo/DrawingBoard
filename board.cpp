#include "board.h"
#include "boardprivate.h"

#include "drawer.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStack>

BoardPrivate::BoardPrivate(Board* _q)
    :q(_q)
{
    backgroundImg = QImage(q->size(), QImage::Format_ARGB32);
    backgroundImg.fill(QColor(0,0,0,1));

    boradImg = QImage(q->size(), QImage::Format_ARGB32);
    // boradImg.fill(QColor(203, 52, 39,50));
    boradImg.fill(Qt::transparent);

    foregroundImg = QImage(q->size(), QImage::Format_ARGB32);
    // foregroundImg.fill(QColor(73, 142, 250,50));
    boradImg.fill(Qt::transparent);

    state = State::READY_TO_DRAW;
    savaState();

    controlPlatform = new Drawer(nullptr, Qt::WindowStaysOnTopHint);
    controlPlatform->resize(300,100);
}

BoardPrivate::~BoardPrivate()
{
    if(controlPlatform)
    {
        delete controlPlatform;
        controlPlatform = nullptr;
    }
}

void BoardPrivate::drawBackgroundImg(QPainter* p)
{
    if(state == State::READY_TO_DRAW)
    {
        p->save();
        p->drawImage(q->rect(),backgroundImg);
        p->restore();
    }
}

void BoardPrivate::drawBoradImg(QPainter* p)
{
    p->save();
    p->drawImage(q->rect(),boradImg);
    p->restore();
}

void BoardPrivate::drawForeGroundImg(QPainter* p)
{
    if(state == State::READY_TO_DRAW)
    {
        p->save();
        p->drawImage(q->rect(),foregroundImg);
        p->restore();
    }
}

void BoardPrivate::savaState()
{
    stateStack.push(state);
}

void BoardPrivate::restoreState()
{
    state = stateStack.isEmpty() ? State::READY_TO_DRAW : stateStack.pop();
}

bool BoardPrivate::showOrHideDrawer(QPoint p)
{
    static bool hideStatus = true;

    auto cRect = q->rect();
    if(p.y() > cRect.center().y())
    {
        if(controlPlatform->isVisible())
        {
            return false;
        }
        else
        {
            controlPlatform->show();
            QPropertyAnimation *anim = new QPropertyAnimation(controlPlatform, "pos", q);
            q->connect(anim, &QPropertyAnimation::finished, q, [anim](){
                anim->deleteLater();
            });
            anim->setDuration(150);
            anim->setStartValue(QPoint(cRect.center().x() - controlPlatform->rect().width() / 2, cRect.bottom()));
            anim->setEndValue(QPoint(cRect.center().x() - controlPlatform->rect().width() / 2, cRect.bottom() - controlPlatform->rect().height()));
            anim->start();
        }

        hideStatus = false;
    }
    else
    {
        if(!controlPlatform->isVisible() || hideStatus)
        {
            hideStatus = true;
            return false;
        }
        else
        {

            hideStatus = true;
            QPropertyAnimation *anim = new QPropertyAnimation(controlPlatform, "pos", q);
            q->connect(anim, &QPropertyAnimation::finished, q, [this, anim](){
                controlPlatform->hide();
                anim->deleteLater();
            });
            anim->setDuration(150);
            anim->setStartValue(QPoint(cRect.center().x() - controlPlatform->rect().width() / 2, cRect.bottom() - controlPlatform->rect().height()));
            anim->setEndValue(QPoint(cRect.center().x() - controlPlatform->rect().width() / 2, cRect.bottom()));
            anim->start();
        }
    }
    return true;
}


Board::Board(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent, f}
    , d(new BoardPrivate(this))
{
    this->setMouseTracking(true);
}

Board::~Board()
{
    if(d)
    {
        delete d;
        d = nullptr;
    }
}


void Board::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    d->drawBackgroundImg(&p);
    d->drawBoradImg(&p);
    d->drawForeGroundImg(&p);

    // drawImage(&p);

    // // auto pix = drawer->currentPen()->shape();
    // // if(!pix.isNull())
    // // {
    // // }
    // if(drawMouseArea)
    // {
    //     p.setBrush(Qt::red);
    //     p.setPen(Qt::transparent);
    //     p.drawRect(mMouseArea);

    //     auto pix = QPixmap(":/res/Icon.png");
    //     auto penPos = QPoint(mousePos.x(), mousePos.y() - pix.size().height());
    //     qDebug() << penPos;
    //         p.drawPixmap(penPos, pix);
    // }
}

void Board::resizeEvent(QResizeEvent* event)
{
    d->backgroundImg = d->backgroundImg.scaled(event->size());
    d->boradImg = d->boradImg.scaled(event->size());
    d->foregroundImg = d->foregroundImg.scaled(event->size());

    QWidget::resizeEvent(event);
}

void Board::mouseMoveEvent(QMouseEvent* event)
{
    auto curPos = event->pos();

    bool upToDate = !d->mouseIsPress ? d->showOrHideDrawer(curPos) : false;

    do{
        static QPoint lastPos;
        if(lastPos.isNull())
        {
            goto DEAW_LINW_END;
        }

        if(d->mouseIsPress)
        {
            drawLine(lastPos, curPos);
        }

DEAW_LINW_END:
            lastPos = curPos;
    }while(0);

    drawPen(curPos);
    upToDate = true;


    if(upToDate)
    {
        this->update();
    }

    QWidget::mouseMoveEvent(event);
}

void Board::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        d->mouseIsPress = true;

        d->controlPlatform->hide();
        // isDraw = true;
        // lastPoint = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void Board::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        d->mouseIsPress = false;
        d->showOrHideDrawer(event->pos());
    }
    else if(event->button() == Qt::RightButton)
    {
        // drawMouseArea = !drawMouseArea;
        // mMouseArea = mouseArea(event->pos());

        // qDebug() << d->state;

        // if(d->state != BoardPrivate::DISPLAY)
        // {
        //     d->state = BoardPrivate::DISPLAY;
        // }
        // else{
        //     d->state = BoardPrivate::READY_TO_DRAW;
        // }

        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void Board::enterEvent(QEnterEvent* event)
{
    d->restoreState();

    update();
}

void Board::leaveEvent(QEvent* event)
{
    d->savaState();
    d->state = BoardPrivate::NOT_READY_TO_DRAW;

    update();
}

void Board::drawLine(QPoint lastMousePos, QPoint mousePos)
{
    QPainter painter(&d->boradImg);
    // 设置画笔属性
    painter.setPen(Qt::green);
    painter.drawLine(lastMousePos, mousePos);
}

void Board::drawPen(QPoint mousePos)
{
    d->foregroundImg.fill(Qt::transparent);

    auto pix = d->controlPlatform->currentPen()->shape();// QPixmap(":/res/Icon.png");

    mousePos.setY(mousePos.y() - pix.height());

    QPainter p(&d->foregroundImg);
    p.drawPixmap(mousePos, pix);
}
