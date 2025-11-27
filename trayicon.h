#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>

class Board;
class SettingView;

class TrayIcon : public QSystemTrayIcon
{
public:
    explicit TrayIcon(QObject *parent = nullptr);

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

public slots:
    void draw();
    void showPreference();

private:
    Board* pBoard = nullptr;
    SettingView* pSettingView = nullptr;
};

#endif // TRAYICON_H
