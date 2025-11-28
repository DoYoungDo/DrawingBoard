#include "dbapplication.h"
#include "trayicon.h"

#include <QHotkey>

#include <QDebug>

int main(int argc, char *argv[])
{
    DBApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 关闭最后一个窗口时不退出应用

    TrayIcon icon;

    QHotkey hotkey(QKeySequence("f4"), true, &a);
    QObject::connect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);

    icon.show();
    return a.exec();
}
