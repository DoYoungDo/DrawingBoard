#ifndef TOOLS_H
#define TOOLS_H

#include <functional>
#include <QMargins>
#include <QUndoStack>

class QBoxLayout;
class QUndoCommand;

namespace TOOLS {
    QBoxLayout* createLayout(Qt::Orientation o, int spacing = 0, const QMargins& m = QMargins(0,0,0,0));

    class UndoRedoCommand : public QUndoCommand
    {
    public:
        UndoRedoCommand(std::function<void ()> undo, std::function<void ()> redo)
            :u(undo),r(redo){}
        virtual void undo() override{ if(u) u();}
        virtual void redo() override{ if(r) r();}
    private:
        std::function<void ()> u = nullptr;
        std::function<void ()> r = nullptr;
    };
    UndoRedoCommand* createUndoRedoCommand(std::function<void(void)> undo, std::function<void(void)> redo);
}

#endif // TOOLS_H
