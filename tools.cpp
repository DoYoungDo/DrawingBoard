#include "tools.h"


QBoxLayout* TOOLS::createLayout(Qt::Orientation o, int spacing, const QMargins& m)
{
    QBoxLayout* layout = new QBoxLayout(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    layout->setSpacing(spacing);
    layout->setContentsMargins(m);
    layout->setSizeConstraint(QBoxLayout::SetNoConstraint);
    return layout;
}
