#ifndef BOARD_H
#define BOARD_H

#include <QWidget>

class Drawer;
class BoardPrivate;

class Board : public QWidget
{
    Q_OBJECT
public:
    explicit Board(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~Board();

    void readyToDraw();

    QPixmap save();
    QPixmap save(bool withBackground);
protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

protected:
    void drawPoint(QPoint pointPos);
    void drawLine(QPoint lastMousePos, QPoint mousePos);
    QRectF drawPen(QPointF mousePos);

private:
    friend class BoardPrivate;
    BoardPrivate* d = nullptr;
};

#endif // BOARD_H
