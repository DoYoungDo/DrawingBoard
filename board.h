#ifndef BOARD_H
#define BOARD_H

#include <QWidget>

class Board : public QWidget
{
    Q_OBJECT
public:
    explicit Board(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

protected:
    virtual void drawImage(QPainter* p);
    virtual void drawLine(QPoint p);

    virtual QRect mouseArea(QPoint p);

private:
    QImage mImage;
    bool drawMouseArea = true;
    QRect mMouseArea;
    bool isDraw = false;
    QPoint lastPoint;
};

#endif // BOARD_H
