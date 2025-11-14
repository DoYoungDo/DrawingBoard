#include "drawer.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QSlider>


ColorButton::ColorButton(const QColor& c, QWidget* parent)
    :QPushButton(parent)
    ,color(c)
{}

void ColorButton::setColor(const QColor& c)
{
    color = c;
    update();
}

void ColorButton::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(color);
    p.drawRect(this->rect());
}

void ColorButton::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit doubleClicked();
}


class DefaultPen : public Pen
{
public:
    DefaultPen()
        :Pen(Qt::SolidPattern, 1, Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin){
    }

    // ~DefaultPen(){}

    QPixmap shape() override{
        return QPixmap(":/res/Icon.png");
    }
};

Drawer::Drawer(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent,Qt::FramelessWindowHint | f}
{
    pensContainer.clear();
    pensContainer << new DefaultPen();

    QLabel* penSizeSliderTitle = new QLabel("size");
    QSlider* penSizeSlider = new QSlider(Qt::Horizontal, this);
    QHBoxLayout* penSizeLayout = new QHBoxLayout;
    penSizeLayout->setContentsMargins(0,0,0,0);
    penSizeLayout->setSpacing(10);
    penSizeLayout->addWidget(penSizeSliderTitle);
    penSizeLayout->addWidget(penSizeSlider);

    ColorButton* defaultColorBottonBL = createColorButton(Qt::black);
    ColorButton* defaultColorBottonW = createColorButton(Qt::white);
    ColorButton* defaultColorBottonR = createColorButton(Qt::red);
    ColorButton* defaultColorBottonY = createColorButton(Qt::yellow);
    ColorButton* defaultColorBottonG = createColorButton(Qt::green);
    ColorButton* defaultColorBottonB = createColorButton(Qt::blue);
    QHBoxLayout* colorButtonLayout = new QHBoxLayout;
    colorButtonLayout->setContentsMargins(0,0,0,0);
    colorButtonLayout->setSpacing(10);
    colorButtonLayout->addWidget(defaultColorBottonBL);
    colorButtonLayout->addWidget(defaultColorBottonW);
    colorButtonLayout->addWidget(defaultColorBottonR);
    colorButtonLayout->addWidget(defaultColorBottonY);
    colorButtonLayout->addWidget(defaultColorBottonG);
    colorButtonLayout->addWidget(defaultColorBottonB);
    colorButtonLayout->addStretch();

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(10);
    mainLayout->addLayout(penSizeLayout,1);
    mainLayout->addLayout(colorButtonLayout,1);
}

Drawer::~Drawer()
{
    if(!pensContainer.isEmpty())
    {
        // for(QPen* pen : pensContainer)
        // {
        //     delete pen;
        //     pen = nullptr;
        // }
        qDeleteAll(pensContainer);
    }
}

Pen* Drawer::currentPen()
{
    return pensContainer.first();
}

void Drawer::onColorButtonClicked(QColor c)
{
    currentPen()->setColor(c);
}

void Drawer::onColorButtonDoubleClicked(ColorButton* btn)
{
    QColor c = QColorDialog::getColor(btn->getColor(), this);
    btn->setColor(c);
}

ColorButton* Drawer::createColorButton(QColor c)
{
    ColorButton* btn = new ColorButton(c);
    btn->setFixedSize(30,30);
    connect(btn, &ColorButton::clicked, this, [btn, this](){
        onColorButtonClicked(btn->getColor());
    });
    connect(btn, &ColorButton::doubleClicked, this, [btn, this](){
        onColorButtonDoubleClicked(btn);
    });
    return btn;
}

// void Drawer::keyPressEvent(QKeyEvent* event)
// {
//     if(event->key() == Qt::Key_Escape)
//     {
//         event->ignore();
//     }
//     QWidget::keyPressEvent(event);
// }
