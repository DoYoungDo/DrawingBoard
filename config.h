#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QObject>
#include <QVariant>

class ConfigHandle;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    ~Config();

    enum ChangedType{INTERNAL,USER};
signals:
    void configChanged(ChangedType type);

protected:
    virtual void timerEvent(QTimerEvent* event) override;

public:
    ConfigHandle* getConfigHandle(ChangedType t);
private:
    void setValue(const QString& id, const QVariant& v);
    QVariant getValue(const QString& id);

    void flush();
private:
    friend class ConfigHandle;
    ConfigHandle* internal = nullptr;
    ConfigHandle* user = nullptr;
    QVariantMap data;
    bool isDirty = false;
    int timerId = -1;
    QFile settingFile;
};

class ConfigHandle : public QObject
{
    Q_OBJECT
    ConfigHandle(Config::ChangedType t, Config* c)
        :QObject(c),type(t),config(c){}
public:
    inline void setValue(const QString& id, const QVariant& v){
        config->setValue(id, v);
        emit config->configChanged(type);
    }
    inline QVariant getValue(const QString& id){return config->getValue(id);}
    inline bool getBool(const QString& id){return getValue(id).toBool();}
    inline int getInt(const QString& id){return getValue(id).toInt();}
    inline double getDouble(const QString& id){return getValue(id).toDouble();}
    inline QString getString(const QString& id){return getValue(id).toString();}
    inline QStringList getStringList(const QString& id){return getValue(id).toStringList();}
    inline QVariantList getVariantList(const QString& id){return getValue(id).toList();}
    inline QVariantMap getVariantMap(const QString& id){return getValue(id).toMap();}

private:
    friend class Config;
    Config::ChangedType type;
    Config* config;
};

#endif // CONFIG_H
