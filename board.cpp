#include "boardprivate.h"
#include "board.h"
#include "drawer.h"
#include "preview.h"
#include "tools.h"
#include "dbapplication.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QStack>
#include <QApplication>
#include <QTimer>
#include <QPen>
#include <QPixmapCache>
#include <QDateTime>
#include <QUndoStack>
#include <QPainterPath>

BoardPrivate::BoardPrivate(Board* _q)
    :q(_q)
{
    state = State::INIT;
    savaState();

    undoredoStack = new QUndoStack(q);

    controlPlatform = new Drawer(q);
    controlPlatform->setVisible(false);
    // controlPlatform->resize(500,250);
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
    controlPlatform->connect(controlPlatform, &Drawer::undoClicked, undoredoStack, &QUndoStack::undo);
    controlPlatform->connect(controlPlatform, &Drawer::redoClicked, undoredoStack, &QUndoStack::redo);
    controlPlatform->connect(controlPlatform, &Drawer::collapsed, controlPlatform, [this](){
        savedControlPlatformGeometry = controlPlatform->geometry();

        QPropertyAnimation *anim = new QPropertyAnimation(controlPlatform, "geometry", q);
        q->connect(anim, &QPropertyAnimation::finished, q, [anim](){
            anim->deleteLater();
        });
        anim->setDuration(100);
        anim->setStartValue(savedControlPlatformGeometry);
        anim->setEndValue(savedControlPlatformGeometry.marginsRemoved(QMargins(0,180,0,0)));
        anim->start();
    });
    controlPlatform->connect(controlPlatform, &Drawer::expanded, controlPlatform, [this](){
        QPropertyAnimation *anim = new QPropertyAnimation(controlPlatform, "geometry", q);
        q->connect(anim, &QPropertyAnimation::finished, q, [anim](){
            anim->deleteLater();
        });
        anim->setDuration(100);
        anim->setStartValue(controlPlatform->geometry());
        anim->setEndValue(savedControlPlatformGeometry);
        anim->start();
    });
    controlPlatform->connect(controlPlatform, &Drawer::downClicked, controlPlatform, [this](){
        if(previewPort)
        {
            previewPort->close();

            delete previewPort;
            previewPort = nullptr;
        }
        QPixmap pic = q->save();
        if(pic.isNull())
        {
            return;
        }

        previewPort = new Preview(pic, q);
        previewPort->setMaxModeSize(q->size());
        previewPort->setMinModeSize(QSize(controlPlatform->width() / 2,controlPlatform->width() / 2));
        previewPort->showMax();

        auto close = [this](bool forceClose = false){
            // qDebug() << "is max" << (previewPort && !previewPort->isMaxMode());
            if(previewPort && (forceClose || !previewPort->isMaxMode())){
                previewPort->close();
                previewPort->deleteLater();
                previewPort = nullptr;
            }
        };

        auto showMin = [this, close](){
            QSize targetSize = previewPort->getMinModeSize();
            QPoint targetPoint(controlPlatform->geometry().right() + 20, controlPlatform->geometry().bottom() - targetSize.height());

            QPropertyAnimation *anim = new QPropertyAnimation(previewPort, "geometry");
            q->connect(anim, &QPropertyAnimation::finished, q, [this, close, anim](){
                anim->deleteLater();

                static QTimer * timer = new QTimer;
                timer->setSingleShot(true);
                if(timer->isActive()){
                    timer->stop();
                }
                timer->callOnTimeout(q, [this, close](){
                    if(previewPort) previewPort->download();
                    close();
                });
                timer->start(1000* 5);
            });
            anim->setDuration(300);
            anim->setStartValue(previewPort->geometry());
            anim->setEndValue(QRect(targetPoint,targetSize));
            anim->start();
        };

        previewPort->connect(previewPort, &Preview::closeButtonClicked, q, [close](){close(true);});
        previewPort->connect(previewPort, &Preview::minButtonClicked, q, showMin);
        previewPort->connect(previewPort, &Preview::doubleClicked, q, [this](){
            if(previewPort->isMaxMode())
            {
                return;
            }

            QSize targetSize = previewPort->getMaxModeSize();
            // QPoint targetPoint(q->geometry().x() + (q->geometry().width() - targetSize.width()) / 2, q->geometry().y() + (q->geometry().height() - targetSize.height()) / 2);
            QPoint targetPoint(0,0);

            QPropertyAnimation *anim = new QPropertyAnimation(previewPort, "geometry");
            q->connect(anim, &QPropertyAnimation::finished, q, [anim](){
                anim->deleteLater();
            });
            anim->setDuration(300);
            anim->setStartValue(previewPort->geometry());
            anim->setEndValue(QRect(targetPoint,targetSize));
            anim->start();
        });

        QTimer::singleShot(300, q, showMin);
    });


    backgroundCanvas = QPixmap(q->size());
    boradCanvas = QPixmap(q->size());
    foregroundCanvas = QPixmap(q->size());

    backgroundCanvas.fill(controlPlatform->backgroundColor());
    boradCanvas.fill(Qt::transparent);
    foregroundCanvas.fill(Qt::transparent);
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
        if(controlPlatform->isVisible() || !(state & State::SHOW_CONTROL) || mouseIsPress)
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
            // QPainter p(&d->boradCanvas);
            // p.setCompositionMode(QPainter::CompositionMode_Clear);
            // p.setPen(Qt::transparent);
            // p.setBrush(Qt::transparent);
            // p.drawRect(d->boradCanvas.rect());

            auto redo = [=](){
                d->boradCanvas.fill(Qt::transparent);
                this->update();
            };

            if(d->undoredoStack)
            {
                QPixmap boradCanvas = d->boradCanvas;
                QUndoCommand* undoCommand = TOOLS::createUndoRedoCommand([this, boradCanvas](){
                    d->boradCanvas = boradCanvas;
                    this->update();
                }, redo);

                d->undoredoStack->push(undoCommand);
            }

            redo();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Board::paintEvent(QPaintEvent* event)
{
    qint64 start =  QDateTime::currentMSecsSinceEpoch();
    // qDebug() << "start" << start;
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    d->drawBackgroundImg(&p);
    d->drawBoradImg(&p);
    d->drawForeGroundImg(&p);

    // qDebug() << "spent" << QDateTime::currentMSecsSinceEpoch() - start;
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
    auto position = event->position();
    if(d->mousePosition == position) return;
    // qDebug() << "last pos" << d->mousePosition << "cur pos" << position;

    d->showOrHideDrawer(position.toPoint());

    QPainterPath path(d->mousePosition);
    path.moveTo(position);
    path.addRect(d->penRectF);

    if(d->mouseLastPos.isNull())
    {
        d->mouseLastPos = position.toPoint();
    }
    else
    {
        if(d->mouseIsPress)
        {
            drawLine(d->mouseLastPos, position.toPoint());
            QPainterPath linePath(d->mouseLastPos);
            linePath.moveTo(position);
            path.addPath(linePath);
        }

        d->mouseLastPos = position.toPoint();
    }

    path.addRect(d->penRectF = drawPen(position));

    QTimer::singleShot(0,[this,path](){
        this->update(path.boundingRect().toRect().marginsAdded(QMargins(100,100,100,100)));
        // this->update();
    });

    d->mousePosition = position;
}

void Board::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        QPixmap boradCanvas = d->boradCanvas;
        d->lastUndo = [this, boradCanvas](){
            d->boradCanvas = boradCanvas;
            this->update();
        };

        d->mouseIsPress = true;
        if(d->state & BoardPrivate::READY_TO_DRAW)
        {
            d->controlPlatform->hide();
            drawPoint(event->position().toPoint());

            this->update();
        }
    }
    else if(event->button() == Qt::BackButton)
    {
        d->undoredoStack->undo();
    }
    else if(event->button() == Qt::ForwardButton)
    {
        d->undoredoStack->redo();
    }
    QWidget::mousePressEvent(event);
}

void Board::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(d->undoredoStack)
        {
            QPixmap boradCanvas = d->boradCanvas;
            QUndoCommand* undoCommand = TOOLS::createUndoRedoCommand(d->lastUndo, [this, boradCanvas](){
                d->boradCanvas = boradCanvas;
                this->update();
            });

            d->undoredoStack->push(undoCommand);
        }

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
    // qDebug() << "enter" << event->position();

    d->mousePosition = event->position();
    d->penRectF = drawPen(d->mousePosition);

    this->repaint();
    // d->mouseLastPos = event->position().toPoint();
    // d->restoreState();
}

void Board::leaveEvent(QEvent* event)
{
    // qDebug() << "leave" << event->position();
    // d->savaState();
    // d->setState((BoardPrivate::State)(d->state & ~BoardPrivate::SHOW_FOREGTOUND));
}

void Board::drawPoint(QPoint pointPos)
{
    if(d->state & BoardPrivate::READY_TO_DRAW)
    {
        const Pen* pen = d->controlPlatform->currentPen();
        QPainter p(&d->boradCanvas);
        p.setOpacity(qreal((qreal)pen->color().alpha() / (qreal)255));
        p.setPen(*pen);
        p.setCompositionMode(pen->isEraser() ? QPainter::CompositionMode_Clear : p.compositionMode());
        p.drawPoint(pointPos);
    }
}

void Board::drawLine(QPoint lastMousePos, QPoint mousePos)
{
    if(d->state & BoardPrivate::READY_TO_DRAW)
    {
        const Pen* pen = d->controlPlatform->currentPen();

        QPainter painter(&d->boradCanvas);
        painter.setOpacity(qreal((qreal)pen->color().alpha() / (qreal)255));
        // 设置画笔属性
        painter.setPen(*d->controlPlatform->currentPen());
        painter.setCompositionMode(pen->isEraser() ? QPainter::CompositionMode_Clear : painter.compositionMode());
        painter.drawLine(lastMousePos, mousePos);
    }
}

QRectF Board::drawPen(QPointF mousePos)
{
    d->foregroundCanvas.fill(Qt::transparent);

    auto pix = d->controlPlatform->currentPen()->shape();

    mousePos.setY(mousePos.y() - pix.height());

    QPainter p(&d->foregroundCanvas);
    p.drawPixmap(mousePos, pix);

    return QRectF(mousePos.x(), mousePos.y(), pix.size().width(), pix.size().height());
}
