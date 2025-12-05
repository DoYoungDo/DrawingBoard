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
"color.palette":[
"#000000","#ffffff","#ff0000","#ffff00","#00ff00","#0000ff"
],
"size.pen":1,
"const.path.setting":"%const.path.setting%",
"const.dir.setting":"%const.dir.setting%",
"dir.download":"%dir.download%",
"language":"\u7b80\u4f53\u4e2d\u6587",
"key.global.draw":"f4",
"download.with.background":false
})";

DBApplication* app = static_cast<DBApplication*>(qApp);
}

Config::Config(QObject *parent)
    : QObject{parent}
    ,internal(new ConfigHandle(INTERNAL, this))
    ,user(new ConfigHandle(USER, this))
    ,settingFile(app->applicationDataDir(true) + "/setting.json")
{
    QString defaultConfig = QString(DEFAULT_CONFIG)
                                .replace("%const.path.setting%",settingFile.fileName())
                                .replace("%const.dir.setting%",app->applicationDataDir())
                                .replace("%dir.download%",app->downloadDir());

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(defaultConfig.toUtf8(), &err);
    Q_ASSERT(err.error == QJsonParseError::NoError);
    Q_ASSERT(doc.isObject());

    data = doc.object().toVariantMap();


    if(settingFile.exists() && settingFile.open(QIODevice::ReadOnly))
    {
        QByteArray content = settingFile.readAll();
        doc = QJsonDocument::fromJson(content, &err);
        if(err.error == QJsonParseError::NoError && doc.isObject())
        {
            // replace default values
            QVariantMap localData = doc.object().toVariantMap();
            for(QString key : localData.keys())
            {
                data.insert(key, localData.value(key));
            }
        }
        settingFile.close();
    }

    // timerId = startTimer(1000);
}

Config::~Config()
{
    if(resetFlag){
        data.clear();
        flush();
    }
    else{
        flush();
        data.clear();
    }
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

void Config::reset()
{
    resetFlag = true;
}

bool Config::setValue(const QString &id, const QVariant &v)
{
    if(id.startsWith("const.")){
        return false;
    }

    data.insert(id, v);
    return true;
}

QVariant Config::getValue(const QString& id)
{
    return data.value(id);
}

void Config::flush()
{
    if(!settingFile.isOpen())
    {
        if(!settingFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "open setting file failed" << QDir(settingFile.fileName()).dirName();

            return;
        }
    }

    QTextStream out(&settingFile);
    out << QJsonDocument(QJsonObject::fromVariantMap(data)).toJson();

    settingFile.close();
}

