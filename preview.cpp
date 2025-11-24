#include "preview.h"
#include "tools.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>


Preview::Preview(const QPixmap& pix, QWidget* parent)
    : QWidget{parent}
{
    this->pix = pix;

    setupUi();
}

void Preview::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);

    p.save();
    p.setBrush(Qt::gray);
    p.drawRect(this->rect());
    p.restore();

    p.drawPixmap(this->rect(), pix);
}

void Preview::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

void Preview::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}

void Preview::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->accept();
    emit doubleClicked();
}

void Preview::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

void Preview::resizeEvent(QResizeEvent* event)
{
    bool isMax = isMaxMode(event->size());
    QList<QPushButton*> btns = this->findChildren<QPushButton*>();
    for(auto btn : btns)
    {
        btn->setVisible(isMax);
    }
    QWidget::resizeEvent(event);
}

void Preview::setMinModeSize(const QSize& s)
{
    minSize = s;
}

QSize Preview::getMinModeSize()
{
    return minSize;
}

void Preview::setMaxModeSize(const QSize& s)
{
    maxSize = s;
}

QSize Preview::getMaxModeSize()
{
    return maxSize;
}

void Preview::showMin()
{
    this->resize(minSize);
    this->show();
}

void Preview::showMax()
{
    this->resize(maxSize);
    this->show();
}

bool Preview::isMaxMode()
{
    return this->size().width() >= maxSize.width() && this->size().height() >= maxSize.height();
}

bool Preview::isMaxMode(const QSize& s)
{
    return s.width() >= maxSize.width() && s.height() >= maxSize.height();
}

void Preview::setupUi()
{
    QBoxLayout* layout = TOOLS::createLayout(Qt::Vertical);
    layout->addLayout(setupToolButtonUi());
    layout->addStretch();
    this->setLayout(layout);
}

QBoxLayout* Preview::setupToolButtonUi()
{
    QPushButton* minButton = new QPushButton("min");
    connect(minButton, &QPushButton::clicked, this, &Preview::minButtonClicked);
    QPushButton* closeButton = new QPushButton("close");
    connect(closeButton, &QPushButton::clicked, this, &Preview::closeButtonClicked);
    QBoxLayout* layout = TOOLS::createLayout(Qt::Horizontal);
    layout->addStretch();
    layout->addWidget(minButton);
    layout->addWidget(closeButton);
    return layout;
}
