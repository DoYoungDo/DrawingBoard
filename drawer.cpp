#include "drawerprivate.h"
#include "drawer.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QSlider>
#include <QBoxLayout>
#include <QDateTime>


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
    repaint();
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


CapabilityButton::CapabilityButton(const QIcon& ic, QWidget* parent)
    :QPushButton(ic,QString(),parent)
{}

void CapabilityButton::paintEvent(QPaintEvent* event)
{
    QIcon ic = this->icon();
    if(ic.isNull())
    {
        QPushButton::paintEvent(event);
        return;
    }

    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);
    p.drawPixmap(this->rect(), ic.pixmap(this->iconSize(), (this->isCheckable() && this->isChecked()) ? QIcon::Selected : QIcon::Normal));
}


class InternalPen : public Pen
{
public:
    InternalPen(const QString& name, const QString& shapeFile, const QString& staticShapeFile,
               const QBrush &brush = Qt::SolidPattern, qreal width = 1, Qt::PenStyle s = Qt::SolidLine,Qt::PenCapStyle c = Qt::RoundCap, Qt::PenJoinStyle j = Qt::RoundJoin)
        :Pen(brush, width, s, c, j)
        ,penName(name),penShapeFile(shapeFile),penStaticShapeFile(staticShapeFile)
    {}
    InternalPen(const InternalPen& pen)
        :Pen(pen)
        ,penName(pen.penName),penShapeFile(pen.penShapeFile),penStaticShapeFile(pen.penStaticShapeFile)
    {}

    QString name() const override{
        return penName;
    }

    QPixmap shape() const override{
        return QPixmap(penShapeFile);
    }

    QPixmap staticShape() const override{
        return QPixmap(penStaticShapeFile);
    }

private:
    QString penName;
    QString penShapeFile;
    QString penStaticShapeFile;
};


DrawerPrivate::DrawerPrivate()
{
    backgroundColor = QColor(0,0,0,1);
}

void DrawerPrivate::addShowOrHide(std::function<void (std::function<void ()>, bool)> showOrHide)
{
    auto oldCollapse = collapse;
    collapse = [oldCollapse,showOrHide](){
        showOrHide(oldCollapse, false);
    };

    auto oldExpand = expand;
    expand = [oldExpand,showOrHide](){
        showOrHide(oldExpand, true);
    };
}


Drawer::Drawer(QWidget *parent, Qt::WindowFlags f)
    : QWidget{parent, f}
    ,d(new DrawerPrivate)
{
    // this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    pensContainer.clear();
    pensContainer << new InternalPen("default",":/res/pens/pen_default.png",":/res/pens/pen_default_static.png")
                  << new InternalPen("pencil",":/res/pens/pen_pencil.png",":/res/pens/pen_pencil_static.png",Qt::SolidPattern,1,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin);
    curPen = pensContainer.first();

    setupUi();
}

Drawer::~Drawer()
{
    if(!pensContainer.isEmpty())
    {
        qDeleteAll(pensContainer);
    }

    if(d)
    {
        delete d;
        d = nullptr;
    }
}

const Pen* Drawer::currentPen()
{
    return curPen;
}

QColor Drawer::backgroundColor() const
{
    return d->backgroundColor;
}

void Drawer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    qint64 start =  QDateTime::currentMSecsSinceEpoch();
    qDebug() << "drawer start" << start;

    QPainter p(this);
    p.setOpacity(0.5);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(255,255,255,120));

    QRect r = this->rect().marginsRemoved(QMargins(1,25,1,1));
    p.drawRoundedRect(r,5,5);


    qDebug() << "drawer spent" << QDateTime::currentMSecsSinceEpoch() - start;
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

void Drawer::collapse()
{
    if(d->collapse)
    {
        d->collapse();
        d->isExpand = false;

        emit collapsed();
    }
}

void Drawer::expand()
{
    if(d->expand)
    {
        d->expand();
        d->isExpand = true;

        emit expanded();
    }
}

void Drawer::setupUi()
{
    QBoxLayout* controlLayout = createLayout(Qt::Horizontal, 10);
    controlLayout->addLayout(setupSliderUi());
    controlLayout->addLayout(setupColorButtonUi());

    QBoxLayout* layout = createLayout(Qt::Vertical, 10, QMargins(10,0,10,10));
    layout->addLayout(setupPenUi(),1);
    layout->addLayout(controlLayout,1);
    layout->addLayout(setupCapabilityButtonUi(), 1);

    this->setLayout(layout);
}

QBoxLayout* Drawer::setupPenUi()
{
    QButtonGroup* penButtonGroup = new QButtonGroup(this);
    penButtonGroup->setExclusive(true);

    QList<PenButton*> penButtons = mapPen<PenButton*>([this,penButtonGroup](Pen* pen)->PenButton*{
        auto btn = createPenButton(pen);
        penButtonGroup->addButton(btn);
        return btn;
    });
    penButtons.first()->setChecked(true);

    QBoxLayout* layout = createLayout(Qt::Horizontal, 10);
    for(auto penBtn : penButtons){
        layout->addWidget(penBtn);
    }
    layout->addStretch();


    d->addShowOrHide([=](std::function<void()> oldCall, bool v){
        if(oldCall){
            oldCall();
        }

        for(auto penBtn : penButtons){
            penBtn->setVisible(v);
        }
    });

    return layout;
}

QBoxLayout* Drawer::setupSliderUi()
{
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

        int alpha = 255 * value /10;
        d->backgroundColor.setAlpha(alpha);
        emit backgroundOpacityChanged(d->backgroundColor);
    });
    QBoxLayout* backgroundAlphaSliderGroupLayout = createLayout(Qt::Vertical,0, QMargins(10,0,10,0));
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
        foreachPen([=](Pen* pen){
            pen->setWidth(value);
        });
        penSizeValueLabel->setText(QString::number(value));
        emit penSizeChanged(value);
    });
    QBoxLayout* penSizeSliderGroupLayout = createLayout(Qt::Vertical,0, QMargins(10,0,10,0));
    penSizeSliderGroupLayout->addWidget(penSizeSlider, 1);
    penSizeSliderGroupLayout->addWidget(penSizeValueLabel, 0);

    QBoxLayout* sliderLayout = createLayout(Qt::Horizontal, 10);
    sliderLayout->addStretch();
    sliderLayout->addLayout(backgroundAlphaSliderGroupLayout);
    sliderLayout->addLayout(penSizeSliderGroupLayout);
    sliderLayout->addStretch();


    d->addShowOrHide([=](std::function<void()> oldCall, bool v){
        if(oldCall){
            oldCall();
        }

        backgroundAlphaSlider->setVisible(v);
        backgroundAlphaValueLabel->setVisible(v);

        penSizeSlider->setVisible(v);
        penSizeValueLabel->setVisible(v);
    });

    return sliderLayout;
}

QBoxLayout* Drawer::setupColorButtonUi()
{
    QRadioButton* backgroundRadioBtn = new QRadioButton("background",this);
    backgroundRadioBtn->setCheckable(true);
    QRadioButton* penRadioBtn = new QRadioButton("pen",this);
    penRadioBtn->setCheckable(true);
    penRadioBtn->setChecked(true);
    QButtonGroup* radioButtonGroup = new QButtonGroup(this);
    radioButtonGroup->setExclusive(true);
    radioButtonGroup->addButton(backgroundRadioBtn, 0);
    radioButtonGroup->addButton(penRadioBtn, 1);

    QBoxLayout* radioButtonLayout = createLayout(Qt::Horizontal, 10);
    radioButtonLayout->addWidget(backgroundRadioBtn);
    radioButtonLayout->addWidget(penRadioBtn);

    auto onColorChanged = [this, radioButtonGroup](const QColor& c){
        if(radioButtonGroup->checkedId() == 0){
            QColor cc = c;
            cc.setAlpha(d->backgroundColor.alpha());
            d->backgroundColor = cc;

            emit backgroundColorChanged(cc);
        }
        else
        {
            emit penColorChanged(c);
        }
    };

    ColorButton* defaultColorBottonBL = createColorButton(Qt::black,onColorChanged);
    defaultColorBottonBL->setChecked(true);
    ColorButton* defaultColorBottonW = createColorButton(Qt::white,onColorChanged);
    ColorButton* defaultColorBottonR = createColorButton(Qt::red,onColorChanged);
    ColorButton* defaultColorBottonY = createColorButton(Qt::yellow,onColorChanged);
    ColorButton* defaultColorBottonG = createColorButton(Qt::green,onColorChanged);
    ColorButton* defaultColorBottonB = createColorButton(Qt::blue,onColorChanged);

    QBoxLayout* colorButtonLayout = createLayout(Qt::Horizontal, 10);
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

    QBoxLayout* colorLayout = createLayout(Qt::Vertical, 10);
    colorLayout->addLayout(radioButtonLayout);
    colorLayout->addLayout(colorButtonLayout);


    d->addShowOrHide([=](std::function<void()> oldCall, bool v){
        if(oldCall){
            oldCall();
        }

        for(auto btn : radioButtonGroup->buttons())
        {
            btn->setVisible(v);
        }

        for(auto btn: colorButtonGroup->buttons())
        {
            btn->setVisible(v);
        }
    });

    return colorLayout;
}

QBoxLayout* Drawer::setupCapabilityButtonUi()
{
    QIcon icon;
    icon.addFile(":/res/icons/arraw_ collapse.svg",QSize(30,30),QIcon::Normal);
    icon.addFile(":/res/icons/arraw_expand.svg",QSize(30,30),QIcon::Selected);
    CapabilityButton* btn = new CapabilityButton(icon, this);
    btn->setFixedSize(30,30);
    btn->setCheckable(true);
    connect(btn, &CapabilityButton::clicked, this, [this](bool checked){
        checked ? collapse() : expand();
    });

    QBoxLayout* layout = createLayout(Qt::Horizontal, 10, QMargins(10, 0, 10, 0));
    layout->addStretch();
    layout->addWidget(btn);
    layout->addStretch();

    return layout;
}

QBoxLayout* Drawer::createLayout(Qt::Orientation o, int spacing, const QMargins& m)
{
    QBoxLayout* layout = new QBoxLayout(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    layout->setSpacing(spacing);
    layout->setContentsMargins(m);
    layout->setSizeConstraint(QBoxLayout::SetMinimumSize);
    return layout;
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

ColorButton* Drawer::createColorButton(QColor c, std::function<void (const QColor&)> colorChangedCb)
{
    ColorButton* btn = new ColorButton(c, this);
    btn->setFixedSize(30,30);
    btn->setCheckable(true);
    connect(btn, &ColorButton::clicked, this, [btn,colorChangedCb, this](){
        auto c = btn->getColor();
        foreachPen([c](Pen*pen){pen->setColor(c);});
        colorChangedCb(c);
    });
    connect(btn, &ColorButton::doubleClicked, this, [btn,colorChangedCb, this](){
        QColor c = QColorDialog::getColor(btn->getColor(), this);
        if(c.isValid())
        {
            btn->setColor(c);

            foreachPen([c](Pen*pen){pen->setColor(c);});
            colorChangedCb(c);
        }
    });
    return btn;
}

void Drawer::foreachPen(std::function<void (Pen*)> cb)
{
    for(Pen* pen : std::as_const(pensContainer))
    {
        cb(pen);
    }
}

Pen* Drawer::findPen(std::function<bool (Pen*)> cb)
{
    for(Pen* pen : std::as_const(pensContainer))
    {
        if(cb(pen)){
            return pen;
        }
    }
    return nullptr;
}


template<typename T>
QList<T> Drawer::mapPen(std::function<T (Pen*)> cb)
{
    QList<T> list;
    for(Pen* pen : std::as_const(pensContainer))
    {
        list << cb(pen);
    }
    return std::move(list);
}

