#ifndef DRAWER_H
#define DRAWER_H

#include "pen.h"

#include <QWidget>
#include <QPushButton>
class QBoxLayout;
class DrawerPrivate;

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

    const Pen* currentPen();
    QColor backgroundColor() const;

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
    void backgroundOpacityChanged(const QColor&);
    void backgroundColorChanged(const QColor&);
    void penSizeChanged(int size);
    void penColorChanged(const QColor&);
    void currentPenChanged(Pen*);
    void collapsed();
    void expanded();
    void downClicked();

public slots:
    void collapse();
    void expand();
private:
    void setupUi();
    QBoxLayout* setupPenUi();
    QBoxLayout* setupSliderUi();
    QBoxLayout* setupColorButtonUi();
    QBoxLayout* setupCapabilityButtonUi();
    QBoxLayout* createLayout(Qt::Orientation o, int spacing = 0, const QMargins& m = QMargins(0,0,0,0));

    PenButton* createPenButton(Pen* p);
    ColorButton* createColorButton(QColor c, std::function<void(const QColor&)> colorChangedCb);
    void foreachPen(std::function<void(Pen*)> cb);
    Pen* findPen(std::function<bool(Pen*)> cb);
    template<typename T>
    QList<T> mapPen(std::function<T(Pen*)> cb);

private:
    QList<Pen*> pensContainer;
    Pen* curPen;

    friend class DrawerPrivate;
    DrawerPrivate* d;
};

#endif // DRAWER_H
