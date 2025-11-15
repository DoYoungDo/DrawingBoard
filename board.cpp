#include "board.h"
#include "boardprivate.h"

#include "drawer.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStack>
#include <QApplication>
#include <QTimer>

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

    state = State::INIT;
    savaState();

    controlPlatform = new Drawer(q);
    controlPlatform->setVisible(false);
    controlPlatform->resize(500,300);
    controlPlatform->installEventFilter(q);
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
    if(state & State::SHOW_BACKGROUND)
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
    if(state & State::SHOW_FOREGTOUND)
    {
        p->save();
        p->drawImage(q->rect(),foregroundImg);
        p->restore();
    }
}

void BoardPrivate::savaState()
{
    qDebug() << "push";
    stateStack.push(state);
}

void BoardPrivate::restoreState()
{
    qDebug() << "pop";
    setState(stateStack.isEmpty() ? State::READY_TO_DRAW : stateStack.pop());
}

void BoardPrivate::setState(State s)
{
    if(state == s)
    {
        return;
    }
    state = s;
    qDebug() << "current state" << s;
    q->update();
}

bool BoardPrivate::showOrHideDrawer(QPoint p)
{
    static bool hideStatus = true;

    auto cRect = q->rect();
    if(p.y() > cRect.center().y())
    {
        if(controlPlatform->isVisible() || !(state & State::SHOW_CONTROL))
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
            anim->setDuration(80);
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
            anim->setDuration(80);
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
    // this->setWindowState(Qt::WindowMaximized);
}

Board::~Board()
{
    if(d)
    {
        delete d;
        d = nullptr;
    }
}

void Board::readyToDraw()
{
    d->setState((BoardPrivate::State)(d->state | BoardPrivate::READY_TO_DRAW));
}

bool Board::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == d->controlPlatform)
    {
        qDebug() << event->type();
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* e = static_cast<QMouseEvent*>(event);
            if(e->button() == Qt::RightButton)
            {
                d->controlPlatform->hide();
                d->setState((BoardPrivate::State)(d->state & ~BoardPrivate::State::SHOW_CONTROL));
                QTimer::singleShot(2000, this,[this](){
                    d->setState((BoardPrivate::State)(d->state | BoardPrivate::State::SHOW_CONTROL));
                });
                return qApp->notify(this,new QMouseEvent(QEvent::MouseMove, this->mapFromGlobal(e->globalPosition()), e->globalPosition(),Qt::NoButton,Qt::NoButton, Qt::NoModifier));
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Board::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    d->drawBackgroundImg(&p);
    d->drawBoradImg(&p);
    d->drawForeGroundImg(&p);
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

    if(d->mouseLastPos.isNull())
    {
        d->mouseLastPos = curPos;
    }
    else
    {
        if(d->mouseIsPress)
        {
            drawLine(d->mouseLastPos, curPos);
        }

        d->mouseLastPos = curPos;
    }

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

        if(d->state & BoardPrivate::READY_TO_DRAW)
        {
            d->controlPlatform->hide();
        }

        // if(d->mouseLastPos != event->position())
        // {
        //     d->mouseLastPos = event->position().toPoint();
        //     drawPen(event->position().toPoint());
        //     update();
        //     return event->accept();
        // }
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
        if(d->state & BoardPrivate::READY_TO_DRAW)
        {
            d->setState((BoardPrivate::State)(d->state & ~BoardPrivate::READY_TO_DRAW));
        }
        else
        {
            d->setState((BoardPrivate::State)(d->state | BoardPrivate::READY_TO_DRAW));
            // 激活当前窗口
            this->raise();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void Board::enterEvent(QEnterEvent* event)
{
    // d->mouseLastPos = event->position().toPoint();
    // d->restoreState();
}

void Board::leaveEvent(QEvent* event)
{
    // d->savaState();
    // d->setState((BoardPrivate::State)(d->state & ~BoardPrivate::SHOW_FOREGTOUND));
}

void Board::drawLine(QPoint lastMousePos, QPoint mousePos)
{
    if(d->state & BoardPrivate::READY_TO_DRAW)
    {
        QPainter painter(&d->boradImg);
        // 设置画笔属性
        painter.setPen(*d->controlPlatform->currentPen());
        painter.drawLine(lastMousePos, mousePos);
    }
}

void Board::drawPen(QPoint mousePos)
{
    d->foregroundImg.fill(Qt::transparent);

    auto pix = d->controlPlatform->currentPen()->shape();// QPixmap(":/res/Icon.png");

    mousePos.setY(mousePos.y() - pix.height());

    QPainter p(&d->foregroundImg);
    p.drawPixmap(mousePos, pix);
}
