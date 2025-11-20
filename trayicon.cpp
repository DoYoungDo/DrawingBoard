#include "trayicon.h"

#include "board.h"
#include "preview.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QTimer>


TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon{parent}
{
    this->setIcon(QIcon(":/icon/res/pens/pen_default.png"));
    this->setToolTip("DrawingBoard");

    QMenu* menu = new QMenu;
    menu->addAction("Draw", [this](){
        if(pBoard && pBoard->isVisible())
        {
            pBoard->raise();
            pBoard->readyToDraw();
            return;
        }

        pBoard = new Board(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        connect(pBoard, &Board::destroyed, this, [this](){pBoard = nullptr;});

        pBoard->setAttribute(Qt::WA_DeleteOnClose, true);
        pBoard->setAttribute(Qt::WA_TranslucentBackground, true);
        pBoard->installEventFilter(this);
        pBoard->showMaximized();
        pBoard->raise();
    });
    menu->addAction("Save", QKeySequence::Save, [this](){
        if(!pBoard)
        {
            return;
        }

        if(pPreview)
        {
            pPreview->deleteLater();
            pPreview = nullptr;
        }

        pPreview = new Preview(pBoard->save());
        pPreview->setWindowFlags(Qt::WindowStaysOnTopHint);
        pPreview->resize(pBoard->size());
        pPreview->show();
        QTimer::singleShot(500,[this](){
            QPropertyAnimation *anim = new QPropertyAnimation(pPreview, "geometry", this);
            anim->setDuration(400);
            anim->setStartValue(pPreview->geometry());
            anim->setEndValue(QRect(pBoard->rect().bottomRight()- QPoint(100, 80), QSize(100, 80)));
            anim->start();
        });
    });

    menu->addSeparator();
    menu->addAction("退出", QKeySequence::Quit, [](){
        qApp->quit();
    });

    this->setContextMenu(menu);
}

bool TrayIcon::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == pBoard)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent)
        {
            switch (keyEvent->key()) {
            case Qt::Key_Escape:
                pBoard->close();
                return true;
            default:
                break;
            }
        }
    }

    return QSystemTrayIcon::eventFilter(watched, event);
}
