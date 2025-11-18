#include "drawer.h"

#include <QButtonGroup>
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

    if(this->isChecked())
    {
        p.save();
        p.setPen(QPen(color, 4));
        p.setBrush(Qt::transparent);

        p.drawRect(this->rect());
        p.restore();

        QRect centerRect = this->rect().marginsRemoved(QMargins(4, 4, 4, 4));
        p.drawRect(centerRect);
    }
    else
    {
        p.drawRect(this->rect());
    }
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
    : QWidget{parent, f}
{
    this->setAttribute( Qt::WA_TranslucentBackground);

    pensContainer.clear();
    pensContainer << new DefaultPen();

    QLabel * backgroundAlphaValueLabel = new QLabel(QString::number(1));
    backgroundAlphaValueLabel->setAlignment(Qt::AlignCenter);
    QSlider* backgroundAlphaSlider = new QSlider(Qt::Vertical, this);
    backgroundAlphaSlider->setToolTip("background");
    backgroundAlphaSlider->setRange(1,10);
    backgroundAlphaSlider->setValue(1);
    backgroundAlphaSlider->setPageStep(1);
    backgroundAlphaSlider->setSingleStep(1);
    connect(backgroundAlphaSlider,&QSlider::valueChanged, this, [this, backgroundAlphaValueLabel](int value){
        backgroundAlphaValueLabel->setText(QString::number(value));
        emit backgroundOpacity(value);
    });
    QVBoxLayout* backgroundAlphaSliderGroupLayout = new QVBoxLayout;
    backgroundAlphaSliderGroupLayout->setContentsMargins(10,0,10,0);
    backgroundAlphaSliderGroupLayout->setSpacing(0);
    backgroundAlphaSliderGroupLayout->addWidget(backgroundAlphaSlider, 1);
    backgroundAlphaSliderGroupLayout->addWidget(backgroundAlphaValueLabel, 0);

    QLabel * penSizeValueLabel = new QLabel(QString::number(1));
    penSizeValueLabel->setAlignment(Qt::AlignCenter);
    QSlider* penSizeSlider = new QSlider(Qt::Vertical, this);
    penSizeSlider->setToolTip("pen size");
    penSizeSlider->setRange(1,100);
    penSizeSlider->setValue(1);
    penSizeSlider->setPageStep(10);
    penSizeSlider->setSingleStep(10);
    connect(penSizeSlider,&QSlider::valueChanged, this, [this, penSizeValueLabel](int value){
        currentPen()->setWidth(value);
        penSizeValueLabel->setText(QString::number(value));
        emit penSizeChanged(value);
    });
    QVBoxLayout* penSizeSliderGroupLayout = new QVBoxLayout;
    penSizeSliderGroupLayout->setContentsMargins(10,0,10,0);
    penSizeSliderGroupLayout->setSpacing(0);
    penSizeSliderGroupLayout->addWidget(penSizeSlider, 1);
    penSizeSliderGroupLayout->addWidget(penSizeValueLabel, 0);


    QHBoxLayout* penSizeLayout = new QHBoxLayout;
    penSizeLayout->setContentsMargins(0,0,0,0);
    penSizeLayout->setSpacing(0);
    penSizeLayout->addLayout(backgroundAlphaSliderGroupLayout);
    penSizeLayout->addLayout(penSizeSliderGroupLayout);
    penSizeLayout->addStretch();

    ColorButton* defaultColorBottonBL = createColorButton(Qt::black);
    defaultColorBottonBL->setChecked(true);
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

    QButtonGroup* colorButtonGroup = new QButtonGroup(this);
    colorButtonGroup->setExclusive(true);
    colorButtonGroup->addButton(defaultColorBottonBL);
    colorButtonGroup->addButton(defaultColorBottonW);
    colorButtonGroup->addButton(defaultColorBottonR);
    colorButtonGroup->addButton(defaultColorBottonY);
    colorButtonGroup->addButton(defaultColorBottonG);
    colorButtonGroup->addButton(defaultColorBottonB);

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
        qDeleteAll(pensContainer);
    }
}

Pen* Drawer::currentPen()
{
    return pensContainer.first();
}

void Drawer::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setOpacity(0.5);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(255,255,255,120));
    p.drawRoundedRect(this->rect().marginsRemoved(QMargins(1,1,1,1)),5,5);

    this->setMask(QRegion(this->rect()));
}

void Drawer::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

void Drawer::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

void Drawer::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}

void Drawer::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->accept();
}

void Drawer::onColorButtonClicked(QColor c)
{
    currentPen()->setColor(c);
    emit penColorChanged(c);
}

void Drawer::onColorButtonDoubleClicked(ColorButton* btn)
{
    QColor c = QColorDialog::getColor(btn->getColor(), this);
    if(c.isValid())
    {
        btn->setColor(c);
    }
}

ColorButton* Drawer::createColorButton(QColor c)
{
    ColorButton* btn = new ColorButton(c);
    btn->setFixedSize(30,30);
    btn->setCheckable(true);
    connect(btn, &ColorButton::clicked, this, [btn, this](){
        onColorButtonClicked(btn->getColor());
    });
    connect(btn, &ColorButton::doubleClicked, this, [btn, this](){
        onColorButtonDoubleClicked(btn);
    });
    return btn;
}


