#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>

class Board;

class TrayIcon : public QSystemTrayIcon
{
public:
    explicit TrayIcon(QObject *parent = nullptr);

public:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Board* pBoard = nullptr;
};

#endif // TRAYICON_H
