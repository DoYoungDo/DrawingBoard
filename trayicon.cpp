#include "trayicon.h"

#include "board.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMenu>


TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon{parent}
{
    this->setIcon(QIcon(":/res/Icon.png"));
    this->setToolTip("DrawingBoard");

    QMenu* menu = new QMenu;
    menu->addAction("Draw", [this](){
        if(pBoard && pBoard->isVisible())
        {
            pBoard->showMaximized();
            return;
        }

        pBoard = new Board(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        connect(pBoard, &Board::destroyed, this, [this](){pBoard = nullptr;});

        pBoard->setAttribute(Qt::WA_DeleteOnClose, true);
        pBoard->setAttribute(Qt::WA_TranslucentBackground, true);
        pBoard->installEventFilter(this);
        // pBoard->show();
        pBoard->showMaximized();
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
