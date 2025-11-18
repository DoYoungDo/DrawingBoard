#ifndef DRAWER_H
#define DRAWER_H

#include "pen.h"

#include <QWidget>
#include <QPushButton>

class PenButton : public QPushButton
{
    Q_OBJECT
public:
    PenButton(Pen* p, QWidget *parent = nullptr);

    Pen* getPen();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
private:
    Pen* pen;
};

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

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
    void backgroundOpacity(int size);
    void penSizeChanged(int size);
    void penColorChanged(const QColor& c);
    void currentPenChanged(Pen*);
private slots:
    void onColorButtonClicked(QColor c);
    void onColorButtonDoubleClicked(ColorButton* btn);
private:
    PenButton* createPenButton(Pen* p);
    ColorButton* createColorButton(QColor c);

private:
    QList<Pen*> pensContainer;
    Pen* curPen;
};

#endif // DRAWER_H
