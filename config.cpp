#include "config.h"
#include "dbapplication.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimerEvent>


namespace {
const char * DEFAULT_CONFIG = R"({
"color.backgroud":"#000000",
"color.backgroud.opacity":1,
"color.pen":"#000000",
"color.pen.opacity":255,
"size.pen":1
})";
}

Config::Config(QObject *parent)
    : QObject{parent}
    ,internal(new ConfigHandle(INTERNAL, this))
    ,user(new ConfigHandle(USER, this))
    ,settingFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/setting.json")
{
    qDebug() << DBApplication::applicationDirPath() << "\n"
             << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) << "\n"
             << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) << "\n"
             << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(DEFAULT_CONFIG, &err);
    Q_ASSERT(err.error == QJsonParseError::NoError);
    Q_ASSERT(doc.isObject());

    data = doc.object().toVariantMap();

    QDir appLocal(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!appLocal.exists())
    {
        appLocal.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    if(settingFilePath.exists() && settingFilePath.open(QIODevice::ReadWrite))
    {
        QByteArray content = settingFilePath.readAll();
        doc = QJsonDocument::fromJson(DEFAULT_CONFIG, &err);
        if(err.error == QJsonParseError::NoError && doc.isObject())
        {
            // replace default values
            QVariantMap localData = doc.object().toVariantMap();
            for(QString key : localData.keys())
            {
                data.insert(key, localData.value(key));
            }
        }
    }

    timerId = startTimer(1000);
}

Config::~Config()
{
    flush();
    data.clear();
    settingFilePath.close();
}

void Config::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == timerId && isDirty)
    {
        flush();
    }
}

ConfigHandle* Config::getConfigHandle(ChangedType t)
{
    switch (t) {
    case USER:
        return user;
    case INTERNAL:
    default:
        return internal;
    }
}

void Config::setValue(const QString& id, const QVariant& v)
{
    data.insert(id, v);
}

QVariant Config::getValue(const QString& id)
{
    return data.value(id);
}

void Config::flush()
{
    if(!settingFilePath.isOpen())
    {
        if(!settingFilePath.open(QIODevice::ReadWrite))
        {
            qDebug() << "open setting file failed" << QDir(settingFilePath.fileName()).dirName();

            return;
        }
    }

    settingFilePath.write(QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
}

