#include "config.h"
#include "dbapplication.h"
#include "trayicon.h"

#include <QHotkey>

#include <QDebug>

namespace {
const char* CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW = "key.global.draw";
}

int main(int argc, char *argv[])
{
    DBApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 关闭最后一个窗口时不退出应用

    TrayIcon icon;

    Config* config = a.getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::INTERNAL);

    QHotkey hotkey(QKeySequence(handle->getString(CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW)), true, &a);
    QObject::connect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);
    QObject::connect(config, &Config::configChanged, &a, [&](Config::ChangedType type, const QString& id){
        // qDebug() << id << type;
        if(id == CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW)
        {
            hotkey.setRegistered(false);
            QObject::disconnect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);

            ConfigHandle* handle = config->getConfigHandle(type);
            QString key = handle->getString(CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW);

            if(key.isEmpty()) return;

            if(hotkey.setShortcut(QKeySequence::fromString(key), true))
            {
                QObject::connect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);
            }
        }
    });

    icon.show();
    return a.exec();
}
