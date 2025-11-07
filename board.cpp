#include "board.h"

#include <QPainter>
#include <QResizeEvent>


Board::Board(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent, f}
{
    mImage = QImage(this->size(), QImage::Format_ARGB32);
    mImage.fill(Qt::transparent);
    mMouseArea = mouseArea(this->cursor().pos());
    this->setMouseTracking(true);
}

void Board::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    drawImage(&p);

    if(drawMouseArea)
    {
        p.setBrush(Qt::red);
        p.setPen(Qt::transparent);
        p.drawRect(mMouseArea);
    }
}

void Board::resizeEvent(QResizeEvent* event)
{
    if(!mImage.isNull())
    {
        QImage newImage(event->size(), QImage::Format_ARGB32);
        newImage.fill(Qt::transparent);
        QPainter painter(&newImage);
        painter.drawImage(0, 0, mImage); // 复制旧图像到新图像
        mImage = newImage;
    }

    QWidget::resizeEvent(event);
}

void Board::mouseMoveEvent(QMouseEvent* event)
{
    bool upToDate = false;

    if(isDraw)
    {
        drawLine(event->pos());
        upToDate = true;
    }

    if(drawMouseArea)
    {
        mMouseArea = mouseArea(event->pos());
        qDebug() << "area" << mMouseArea;
        upToDate = true;
    }


    if(upToDate)
    {
        this->update();
    }

    QWidget::mouseMoveEvent(event);
}

void Board::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        isDraw = true;
        lastPoint = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void Board::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        isDraw = false;
    }
    else if(event->button() == Qt::RightButton)
    {
        drawMouseArea = !drawMouseArea;
        mMouseArea = mouseArea(event->pos());
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void Board::drawImage(QPainter* p)
{
    if(!mImage.isNull())
    {
        p->drawImage(0, 0, mImage);
    }
}

void Board::drawLine(QPoint p)
{
    QPainter painter(&mImage);
    // 设置画笔属性
    // QPen pen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(Qt::green);
    // 绘制从lastPoint到endPoint的线段（形成连续轨迹）
    // painter.drawPoint(p);
    painter.drawLine(lastPoint, p);

    lastPoint = p;
}

QRect Board::mouseArea(QPoint p)
{
    const int pix = 10;
    return QRect(QPoint(p.x() - pix, p.y() - pix),QSize(pix*2, pix*2));
}
