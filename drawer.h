#ifndef DRAWER_H
#define DRAWER_H

#include "pen.h"

#include <QWidget>
#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton(const QColor& c, QWidget *parent = nullptr);

    void setColor(const QColor& c);
    QColor getColor(){return color;}

signals:
    void doubleClicked();
protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
    QColor color;
};

class Drawer : public QWidget
{
    Q_OBJECT
public:
    explicit Drawer(QWidget *parent = nullptr,  Qt::WindowFlags f = Qt::WindowFlags());
    ~Drawer();

    Pen* currentPen();

    // QWidget interface
protected:
    // virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onColorButtonClicked(QColor c);
    void onColorButtonDoubleClicked(ColorButton* btn);
private:
    ColorButton* createColorButton(QColor c);

private:
    QList<Pen*> pensContainer;
};

#endif // DRAWER_H
