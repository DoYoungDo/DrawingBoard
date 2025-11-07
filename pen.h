#ifndef PEN_H
#define PEN_H

#include <QObject>

class Pen : public QObject
{
    Q_OBJECT
public:
    explicit Pen(QObject *parent = nullptr);

signals:

};

#endif // PEN_H
