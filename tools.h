#ifndef TOOLS_H
#define TOOLS_H

#include <QBoxLayout>
#include <QMargins>

namespace TOOLS {
    QBoxLayout* createLayout(Qt::Orientation o, int spacing = 0, const QMargins& m = QMargins(0,0,0,0));
}

#endif // TOOLS_H
