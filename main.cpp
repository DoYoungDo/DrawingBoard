#include "config.h"
#include "dbapplication.h"
#include "translator.h"
#include "trayicon.h"

#include <QHotkey>

#include <QDebug>

namespace {
const char* CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW = "key.global.draw";
const char* CONFIG_LANGUAGE = "language";
}

int main(int argc, char *argv[])
{
    DBApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 关闭最后一个窗口时不退出应用

    Config* config = a.getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::INTERNAL);

    Translator* translator = new Translator(handle->getString(CONFIG_LANGUAGE), &a);
    a.installTranslator(translator);

    TrayIcon icon;

    QHotkey hotkey(QKeySequence(handle->getString(CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW)), true, &a);
    if(hotkey.isRegistered())
    {
        QObject::connect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);
    }

    QObject::connect(config, &Config::configChanged, &a, [&](Config::ChangedType type, const QString& id){
        // qDebug() << id << type;
        ConfigHandle* handle = config->getConfigHandle(type);

        if(id == CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW)
        {
            hotkey.setRegistered(false);
            QObject::disconnect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);

            QString key = handle->getString(CONFIG_KEY_SHORT_CUT_GLOBAL_DRAW);

            if(key.isEmpty()) return;

            if(hotkey.setShortcut(QKeySequence::fromString(key), true))
            {
                QObject::connect(&hotkey, &QHotkey::activated, &icon, &TrayIcon::draw);
            }
        }
        else if(id == CONFIG_LANGUAGE)
        {
            QString lan = handle->getString(CONFIG_LANGUAGE);
            a.removeTranslator(translator);
            // translator->deleteLater();
            delete translator;
            translator = new Translator(lan, &a);
            a.installTranslator(translator);
        }
    });

    icon.show();
    return a.exec();
}
