#ifndef CAPABILITYBUTTON_H
#define CAPABILITYBUTTON_H

#include <QPushButton>

class CapabilityButton : public QPushButton
{
    Q_OBJECT
public:
    CapabilityButton(const QIcon& ic, QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
};

#endif // CAPABILITYBUTTON_H
