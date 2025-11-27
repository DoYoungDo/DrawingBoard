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
};

#endif // CAPABILITYBUTTON_H
