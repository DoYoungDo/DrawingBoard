#include "drawer.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QSlider>



PenButton::PenButton(Pen* p, QWidget* parent)
    :QPushButton(parent)
    ,pen(p)
{}

Pen* PenButton::getPen()
{
    return pen;
}

void PenButton::paintEvent(QPaintEvent* event)
{
    Q_ASSERT(pen);

    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);

    QRect areaRect = this->rect().marginsRemoved(this->isChecked() ? QMargins(0,0,0,10) : QMargins(0,10,0,0));
    p.drawPixmap(areaRect, pen->staticShape());
}


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

    QString name() override{
        return "default";
    }

    QPixmap shape() override{
        return QPixmap(":/res/Icon.png");
    }

    QPixmap staticShape() override{
        return QPixmap(":/res/staticIcon.png");
    }
};


Drawer::Drawer(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent, f}
{
    // this->setAttribute( Qt::WA_TranslucentBackground);

    pensContainer.clear();
    pensContainer << new DefaultPen() << new DefaultPen;
    curPen = pensContainer.first();

    QList<PenButton*> penButtons;
    QButtonGroup* penButtonGroup = new QButtonGroup(this);
    penButtonGroup->setExclusive(true);
    for(auto pen : pensContainer)
    {
        auto btn = createPenButton(pen);

        penButtonGroup->addButton(btn);
        penButtons << btn;
    }

    penButtons.first()->setChecked(true);

    QHBoxLayout* penButtonLayout = new QHBoxLayout;
    penButtonLayout->setContentsMargins(10,0,10,0);
    penButtonLayout->setSpacing(10);
    for(auto penBtn : penButtons){
        penButtonLayout->addWidget(penBtn);
    }
    penButtonLayout->addStretch();

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


    QHBoxLayout* sliderLayout = new QHBoxLayout;
    sliderLayout->setContentsMargins(0,0,0,0);
    sliderLayout->setSpacing(0);
    sliderLayout->addLayout(backgroundAlphaSliderGroupLayout);
    sliderLayout->addLayout(penSizeSliderGroupLayout);
    sliderLayout->addStretch();

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

    QHBoxLayout* BottomLayout = new QHBoxLayout;
    BottomLayout->setContentsMargins(0,0,0,0);
    BottomLayout->setSpacing(10);
    BottomLayout->addLayout(sliderLayout,1);
    BottomLayout->addLayout(colorButtonLayout,1);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(penButtonLayout,1);
    mainLayout->addLayout(BottomLayout,1);
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
    return curPen;
}

void Drawer::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setOpacity(0.5);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(255,255,255,120));

    QRect r = this->rect().marginsRemoved(QMargins(1,25,1,1));
    p.drawRoundedRect(r,5,5);

    // this->setMask(QRegion(r));
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

PenButton* Drawer::createPenButton(Pen* p)
{
    PenButton* btn = new PenButton(p, this);
    btn->setFixedSize(30,90);
    btn->setCheckable(true);
    connect(btn, &PenButton::clicked, this, [btn, this](){
        curPen = btn->getPen();
        emit currentPenChanged(curPen);
    });
    return btn;
}

ColorButton* Drawer::createColorButton(QColor c)
{
    ColorButton* btn = new ColorButton(c, this);
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

