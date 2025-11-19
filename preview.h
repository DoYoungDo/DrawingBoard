#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>

class Preview : public QWidget
{
    Q_OBJECT
public:
    explicit Preview(const QPixmap& pix, QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    QPixmap pix;
};

#endif // PREVIEW_H
