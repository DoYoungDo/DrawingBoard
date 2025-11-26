#ifndef DBAPPLICATION_H
#define DBAPPLICATION_H

#include <QApplication>

class DBApplication : public QApplication
{
public:
    DBApplication(int &argc, char **argv);

    template<typename T>
    T* getSingleton()
    {
        QString name = typeid(T).name();
        // qDebug() << "type id" << name;
        if(singletons.contains(name))
        {
            return static_cast<T*>(singletons.value(name));
        }
        return nullptr;
    }

    template<typename T>
    bool registerSingleton(T* ins)
    {
        QString name = typeid(T).name();
        // qDebug() << "type id" << name;
        if(singletons.contains(name))
        {
            return false;
        }
        singletons.insert(name, ins);
        return true;
    }


    QString applicationDataDir(bool mk = false);
    QString downloadDir();

private:
    QMap<QString, void *> singletons;
};

#endif // DBAPPLICATION_H
