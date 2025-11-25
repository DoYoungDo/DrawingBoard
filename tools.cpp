#include "tools.h"

#include <QBoxLayout>
#include <QUndoCommand>


QBoxLayout* TOOLS::createLayout(Qt::Orientation o, int spacing, const QMargins& m)
{
    QBoxLayout* layout = new QBoxLayout(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    layout->setSpacing(spacing);
    layout->setContentsMargins(m);
    layout->setSizeConstraint(QBoxLayout::SetNoConstraint);
    return layout;
}

TOOLS::UndoRedoCommand* TOOLS::createUndoRedoCommand(std::function<void ()> undo, std::function<void ()> redo)
{
    return new UndoRedoCommand(undo, redo);
}
