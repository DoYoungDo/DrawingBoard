#include "config.h"
#include "dbapplication.h"

#include <QDir>
#include <QStandardPaths>
#include <QUndoStack>


DBApplication::DBApplication(int& argc, char** argv)
    :QApplication(argc, argv)
{
    registerSingleton(new QUndoStack(this));
    registerSingleton(new Config(this));
}

QString DBApplication::applicationDataDir(bool mk)
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(mk)
    {
        QDir d(dir);
        if(!d.exists())
        {
            d.mkpath(dir);
        }
    }
    return dir;
}
