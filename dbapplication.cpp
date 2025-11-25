#include "dbapplication.h"

#include <QUndoStack>


DBApplication::DBApplication(int& argc, char** argv)
    :QApplication(argc, argv)
{
    registerSingleton(new QUndoStack(this));
}
