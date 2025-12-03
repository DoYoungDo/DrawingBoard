#include "trayicon.h"

#include "board.h"
#include "preview.h"
#include "settingview.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QTimer>


TrayIcon::TrayIcon(QObject *parent)
    : QSystemTrayIcon{parent}
{
    this->setIcon(QIcon(":/icon/res/icon.png"));
    this->setToolTip("DrawingBoard");

    QMenu* menu = new QMenu;
    QAction* drawAction = menu->addAction(tr("menu.action.text.draw"));
    connect(drawAction, &QAction::triggered, this, &TrayIcon::draw);
    QAction* preferenceAction = menu->addAction(tr("menu.action.text.preference"), QKeySequence::Preferences);
    connect(preferenceAction, &QAction::triggered, this, &TrayIcon::showPreference);
    menu->addSeparator();
    menu->addAction(tr("menu.action.text.quit"), QKeySequence::Quit, [](){
        qApp->quit();
    });

    this->setContextMenu(menu);
}

bool TrayIcon::eventFilter(QObject* watched, QEvent* event)
{
    if(watched->inherits("QWidget"))
    {
        QWidget* w = qobject_cast<QWidget*>(watched);
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent)
        {
            switch (keyEvent->key()) {
            case Qt::Key_Escape:
                w->close();
                return true;
            default:
                break;
            }
        }
    }

    return QSystemTrayIcon::eventFilter(watched, event);
}

void TrayIcon::draw()
{
    if(pSettingView && pSettingView->isVisible())
    {
        return;
    }

    if(pBoard && pBoard->isVisible())
    {
        pBoard->raise();
        pBoard->readyToDraw();
        return;
    }

#ifdef Q_OS_WIN
    pBoard = new Board(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);
#else
    pBoard = new Board(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#endif
    connect(pBoard, &Board::destroyed, this, [this](){pBoard = nullptr;});

    pBoard->setAttribute(Qt::WA_DeleteOnClose, true);
    pBoard->setAttribute(Qt::WA_TranslucentBackground, true);
    pBoard->installEventFilter(this);
    pBoard->showMaximized();
    pBoard->raise();
}

void TrayIcon::showPreference()
{
    if(pSettingView)
    {
        pSettingView->show();
        return;
    }

    bool boardVisible = pBoard && pBoard->isVisible();
    if(boardVisible)
    {
        pBoard->hide();
    }

    pSettingView = new SettingView();
    pSettingView->installEventFilter(this);
    pSettingView->setAttribute(Qt::WA_DeleteOnClose, true);
    pSettingView->show();

    connect(pSettingView, &SettingView::destroyed, this, [this, boardVisible](){
        pSettingView = nullptr;
        if(boardVisible){
            pBoard->show();
        }
    });
}
