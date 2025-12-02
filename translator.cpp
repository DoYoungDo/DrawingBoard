#include "translator.h"

#include "config.h"
#include "dbapplication.h"

#include <QJsonDocument>
#include <QJsonObject>


Translator::Translator(const QString &lanName, QObject *parent)
    :QTranslator(parent)
{
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);

    QFile file(QString(":/i18n/%1.json").arg(lanName));
    if(!file.exists())
    {
        file.setFileName(":/i18n/\u7b80\u4f53\u4e2d\u6587.json");
        Q_ASSERT(file.exists());
    }

    if(file.open(QIODevice::ReadOnly)){
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
        if(err.error == QJsonParseError::NoError)
        {
            QJsonObject obj = doc.object();
            for(const QString& k : obj.keys())
            {
                words.insert(k, obj.value(k).toString(""));
            }
        }
    }
}

QString Translator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    Q_UNUSED(context);
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);
    if(words.contains(sourceText))
    {
        return words.value(sourceText);
    }
    return sourceText;
}

bool Translator::isEmpty() const
{
    return words.isEmpty();
}
