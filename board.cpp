#include "boardprivate.h"
#include "board.h"
#include "drawer.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStack>
#include <QApplication>
#include <QTimer>
#include <QPen>
#include <QPixmapCache>
#include <QDateTime>

BoardPrivate::BoardPrivate(Board* _q)
    :q(_q)
{
    state = State::INIT;
    savaState();

    controlPlatform = new Drawer(q);
    controlPlatform->setVisible(false);
    controlPlatform->resize(500,200);
    controlPlatform->installEventFilter(q);

    auto updateBackground = [this](const QColor& c){
        qDebug() << "backgroud color" << c;
        backgroundCanvas.fill(c);
        q->update();
    };
    controlPlatform->connect(controlPlatform, &Drawer::backgroundOpacityChanged, controlPlatform, updateBackground);
    controlPlatform->connect(controlPlatform, &Drawer::backgroundColorChanged, controlPlatform, updateBackground);
    controlPlatform->connect(controlPlatform, &Drawer::penSizeChanged, controlPlatform, [this](int value){
        foregroundCanvas.fill(Qt::transparent);

        QPainter p(&foregroundCanvas);
        p.setPen(*controlPlatform->currentPen());
        p.drawPoint(q->rect().center());

        q->update();
    });
    controlPlatform->connect(controlPlatform, &Drawer::penColorChanged, controlPlatform, [this](const QColor& c){
        foregroundCanvas.fill(Qt::transparent);

        QPainter p(&foregroundCanvas);
        QPen pen = *controlPlatform->currentPen();
        pen.setWidth(50);
        p.setPen(pen);
        p.drawPoint(q->rect().center());

        q->update();
    });

    backgroundCanvas = QPixmap(q->size());
    boradCanvas = QPixmap(q->size());
    foregroundCanvas = QPixmap(q->size());

    backgroundCanvas.fill(controlPlatform->backgroundColor());
    boradCanvas.fill(Qt::transparent);
    boradCanvas.fill(Qt::transparent);
}

BoardPrivate::~BoardPrivate()
{
    if(controlPlatform)
    {
        delete controlPlatform;
        controlPlatform = nullptr;
    }

    QPixmapCache::clear();
}

void BoardPrivate::drawBackgroundImg(QPainter* p)
{
    if(state & State::SHOW_BACKGROUND)
    {
        p->save();
        p->drawPixmap(q->rect(),backgroundCanvas);
        p->restore();
    }
}

void BoardPrivate::drawBoradImg(QPainter* p)
{
    p->save();
    p->drawPixmap(q->rect(),boradCanvas);
    p->restore();
}

void BoardPrivate::drawForeGroundImg(QPainter* p)
{
    if(state & State::SHOW_FOREGTOUND)
    {
        p->save();
        p->drawPixmap(q->rect(),foregroundCanvas);
        p->restore();
    }
}

void BoardPrivate::savaState()
{
    // qDebug() << "push";
    stateStack.push(state);
}

void BoardPrivate::restoreState()
{
    // qDebug() << "pop";
    setState(stateStack.isEmpty() ? State::READY_TO_DRAW : stateStack.pop());
}

void BoardPrivate::setState(State s)
{
    if(state == s)
    {
        return;
    }
    state = s;
    // qDebug() << "current state" << s;
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

    this->setAttribute(Qt::WA_OpaquePaintEvent);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAutoFillBackground(false);

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

void Board::readyToDraw()
{
    d->setState((BoardPrivate::State)(d->state | BoardPrivate::READY_TO_DRAW));
}

QPixmap Board::save()
{
    return d->boradCanvas;
}

bool Board::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == d->controlPlatform)
    {
        // qDebug() << event->type();
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
        else if(event->type() == QEvent::MouseButtonDblClick)
        {
            d->boradCanvas.fill(Qt::transparent);
            this->update();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Board::paintEvent(QPaintEvent* event)
{
    qint64 start =  QDateTime::currentMSecsSinceEpoch();
    qDebug() << "start" << start;
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    d->drawBackgroundImg(&p);
    d->drawBoradImg(&p);
    d->drawForeGroundImg(&p);

    qDebug() << "spent" << QDateTime::currentMSecsSinceEpoch() - start;
}

void Board::resizeEvent(QResizeEvent* event)
{
    d->backgroundCanvas = d->backgroundCanvas.scaled(event->size());
    d->boradCanvas = d->boradCanvas.scaled(event->size());
    d->foregroundCanvas = d->foregroundCanvas.scaled(event->size());

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
            drawPoint(event->position().toPoint());

            this->update();
        }
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

void Board::drawPoint(QPoint pointPos)
{
    if(d->state & BoardPrivate::READY_TO_DRAW)
    {
        QPainter p(&d->boradCanvas);
        p.setPen(*d->controlPlatform->currentPen());
        p.drawPoint(pointPos);
    }
}

void Board::drawLine(QPoint lastMousePos, QPoint mousePos)
{
    if(d->state & BoardPrivate::READY_TO_DRAW)
    {
        QPainter painter(&d->boradCanvas);
        // 设置画笔属性
        painter.setPen(*d->controlPlatform->currentPen());
        painter.drawLine(lastMousePos, mousePos);
    }
}

void Board::drawPen(QPoint mousePos)
{
    d->foregroundCanvas.fill(Qt::transparent);

    auto pix = d->controlPlatform->currentPen()->shape();

    mousePos.setY(mousePos.y() - pix.height());

    QPainter p(&d->foregroundCanvas);
    p.drawPixmap(mousePos, pix);
}
