#include "dbapplication.h"
#include "trayicon.h"

int main(int argc, char *argv[])
{
    DBApplication a(argc, argv);
    // 关键：设置应用为后台应用（不显示程序坞图标）
    // a.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    a.setQuitOnLastWindowClosed(false); // 关闭最后一个窗口时不退出应用

    TrayIcon icon;
    icon.show();
    return a.exec();
}
