#include "drawerprivate.h"
#include "drawer.h"
#include "dbapplication.h"
#include "tools.h"
#include "config.h"

#include <capabilitybutton.h>

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
#include <QLineEdit>
#include <QSpinBox>


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


class InternalPen : public Pen
{
public:
    InternalPen(const QString& name, const QString& shapeFile, const QString& staticShapeFile, bool isEraser = false,
               const QBrush &brush = Qt::SolidPattern, qreal width = 1, Qt::PenStyle s = Qt::SolidLine,Qt::PenCapStyle c = Qt::RoundCap, Qt::PenJoinStyle j = Qt::RoundJoin)
        :Pen(brush, width, s, c, j)
        ,penName(name),penShapeFile(shapeFile),penStaticShapeFile(staticShapeFile), isEr(isEraser)
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

    bool isEraser() const override{
        return isEr;
    }

private:
    QString penName;
    QString penShapeFile;
    QString penStaticShapeFile;
    bool isEr = false;
};


DrawerPrivate::DrawerPrivate()
{
    ConfigHandle* handle = static_cast<DBApplication*>(qApp)->getSingleton<Config>()->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);

    backgroundColor = QColor(handle->getString("color.backgroud"));
    int o =handle->getInt("color.backgroud.opacity");
    backgroundColor.setAlpha(o == 1 ? o : 255*o/10);
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
    pensContainer.clear();
    pensContainer
            // << new InternalPen("default",":/res/pens/pen_default.png",":/res/pens/pen_default_static.png")
            << new InternalPen("pencil",":/res/pens/pen_pencil.png",":/res/pens/pen_pencil_static.png"/*, false ,Qt::SolidPattern,1,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin*/)
            << new InternalPen("eraser",":/res/pens/eraser.png",":/res/pens/eraser_static.png", true);
    curPen = pensContainer.first();

    setupUi();

    ConfigHandle* handle = static_cast<DBApplication*>(qApp)->getSingleton<Config>()->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);

    foreachPen([=](Pen* pen){
        pen->setWidth(handle->getInt("size.pen"));

        QColor c(handle->getString("color.pen"));
        c.setAlpha(handle->getInt("color.pen.opacity"));
        pen->setColor(c);
    });
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
    // qDebug() << "drawer start" << start;

    QPainter p(this);
    p.setOpacity(0.5);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(125,125,125,120));

    QRect r = this->rect().marginsRemoved(QMargins(1,d->isExpand ? 25 : 1,1,1));
    p.drawRoundedRect(r,5,5);


    // qDebug() << "drawer spent" << QDateTime::currentMSecsSinceEpoch() - start;
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

void Drawer::leaveEvent(QEvent *event)
{
    emit leave();
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
        emit expanded();

        d->expand();
        d->isExpand = true;
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
    layout->addStretch();
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
    ConfigHandle* handle = static_cast<DBApplication*>(qApp)->getSingleton<Config>()->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);

    QSpinBox* backgroundAlphaValueEdit = new QSpinBox(this);
    backgroundAlphaValueEdit->setRange(1, 10);
    backgroundAlphaValueEdit->setValue(handle->getInt("color.backgroud.opacity"));
    backgroundAlphaValueEdit->setSingleStep(1);
    QSlider* backgroundAlphaSlider = new QSlider(Qt::Vertical, this);
    backgroundAlphaSlider->setToolTip("background");
    backgroundAlphaSlider->setRange(1,10);
    backgroundAlphaSlider->setValue(backgroundAlphaValueEdit->value());
    backgroundAlphaSlider->setPageStep(1);
    backgroundAlphaSlider->setSingleStep(1);
    connect(backgroundAlphaSlider,&QSlider::valueChanged, this, [this, handle, backgroundAlphaValueEdit](int value){
        handle->setValue("color.backgroud.opacity", value);

        int alpha = value == 1 ? value : 255 * value /10;
        d->backgroundColor.setAlpha(alpha);

        blockSignals(true);
        backgroundAlphaValueEdit->setValue(value);
        blockSignals(false);
        emit backgroundOpacityChanged(d->backgroundColor);
    });
    connect(backgroundAlphaValueEdit, &QSpinBox::valueChanged, backgroundAlphaSlider, &QSlider::setValue);
    QBoxLayout* backgroundAlphaSliderGroupLayout = createLayout(Qt::Vertical,0, QMargins(10,0,10,0));
    backgroundAlphaSliderGroupLayout->addWidget(backgroundAlphaSlider, 1);
    backgroundAlphaSliderGroupLayout->addWidget(backgroundAlphaValueEdit, 0);

    QSpinBox* penSizeEdit = new QSpinBox(this);
    penSizeEdit->setRange(1, 100);
    penSizeEdit->setValue(handle->getInt("size.pen"));
    penSizeEdit->setSingleStep(1);
    QSlider* penSizeSlider = new QSlider(Qt::Vertical, this);
    penSizeSlider->setToolTip("pen size");
    penSizeSlider->setRange(1,100);
    penSizeSlider->setValue(penSizeEdit->value());
    penSizeSlider->setPageStep(10);
    penSizeSlider->setSingleStep(10);
    connect(penSizeSlider,&QSlider::valueChanged, this, [this, handle, penSizeEdit](int value){
        handle->setValue("size.pen", value);

        foreachPen([=](Pen* pen){
            pen->setWidth(value);
        });

        blockSignals(true);
        penSizeEdit->setValue(value);
        blockSignals(false);

        emit penSizeChanged(value);
    });
    connect(penSizeEdit, &QSpinBox::valueChanged, penSizeSlider, &QSlider::setValue);
    QBoxLayout* penSizeSliderGroupLayout = createLayout(Qt::Vertical,0, QMargins(10,0,10,0));
    penSizeSliderGroupLayout->addWidget(penSizeSlider, 1);
    penSizeSliderGroupLayout->addWidget(penSizeEdit, 0);

    QSpinBox* penAlphaEdit = new QSpinBox(this);
    penAlphaEdit->setRange(1, 255);
    penAlphaEdit->setValue(handle->getInt("color.pen.opacity"));
    penAlphaEdit->setSingleStep(1);
    QSlider* penAlphaSlider = new QSlider(Qt::Vertical, this);
    penAlphaSlider->setToolTip("pen opacity");
    penAlphaSlider->setRange(1,255);
    penAlphaSlider->setValue(penAlphaEdit->value());
    penAlphaSlider->setPageStep(10);
    penAlphaSlider->setSingleStep(10);
    connect(penAlphaSlider,&QSlider::valueChanged, this, [this, handle, penAlphaEdit](int value){
        handle->setValue("color.pen.opacity", value);

        foreachPen([=](Pen* pen){
            QColor c = pen->color();
            c.setAlpha(value);
            pen->setColor(c);
        });

        blockSignals(true);
        penAlphaEdit->setValue(value);
        blockSignals(false);
    });
    connect(penAlphaEdit, &QSpinBox::valueChanged, penAlphaSlider, &QSlider::setValue);
    QBoxLayout* penAlphaSliderGroupLayout = createLayout(Qt::Vertical,0, QMargins(10,0,10,0));
    penAlphaSliderGroupLayout->addWidget(penAlphaSlider, 1);
    penAlphaSliderGroupLayout->addWidget(penAlphaEdit, 0);

    QBoxLayout* sliderLayout = createLayout(Qt::Horizontal, 10);
    sliderLayout->addStretch();
    sliderLayout->addLayout(backgroundAlphaSliderGroupLayout);
    sliderLayout->addLayout(penSizeSliderGroupLayout);
    sliderLayout->addLayout(penAlphaSliderGroupLayout);
    sliderLayout->addStretch();

    d->addShowOrHide([=](std::function<void()> oldCall, bool v){
        if(oldCall){
            oldCall();
        }

        backgroundAlphaSlider->setVisible(v);
        backgroundAlphaValueEdit->setVisible(v);

        penSizeSlider->setVisible(v);
        penSizeEdit->setVisible(v);

        penAlphaSlider->setVisible(v);
        penAlphaEdit->setVisible(v);
    });

    return sliderLayout;
}

QBoxLayout* Drawer::setupColorButtonUi()
{
    ConfigHandle* handle = static_cast<DBApplication*>(qApp)->getSingleton<Config>()->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);

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


    QButtonGroup* colorButtonGroup = new QButtonGroup(this);
    colorButtonGroup->setExclusive(true);

    QBoxLayout* colorButtonLayout = createLayout(Qt::Horizontal, 10);

    auto onColorChanged = [this,handle, radioButtonGroup, colorButtonGroup](const QColor& c){
        QStringList colors;
        for(auto btn : colorButtonGroup->buttons())
        {
            colors << static_cast<ColorButton*>(btn)->getColor().name();
        }
        // qDebug() << "colors " << colors;
        handle->setValue("color.palette", colors);

        if(radioButtonGroup->checkedId() == 0){
            handle->setValue("color.backgroud", c.name());

            QColor cc = c;
            cc.setAlpha(d->backgroundColor.alpha());
            d->backgroundColor = cc;

            emit backgroundColorChanged(cc);
        }
        else
        {
            handle->setValue("color.pen", c.name());
            emit penColorChanged(c);
        }
    };

    QString penColor = handle->getString("color.pen");
    QStringList colors = handle->getStringList("color.palette");
    for(const QString& c : colors)
    {
        ColorButton* colorButton = createColorButton(c,onColorChanged);
        if(penColor == c)
        {
            colorButton->setChecked(true);
        }

        colorButtonGroup->addButton(colorButton);
        colorButtonLayout->addWidget(colorButton);
    }

    colorButtonLayout->addStretch();

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

        colorLayout->setSpacing(v ? 10 : 0);
    });

    return colorLayout;
}

QBoxLayout* Drawer::setupCapabilityButtonUi()
{
    QIcon iconUndo;
    iconUndo.addFile(":/res/icons/undo.png",QSize(32,32),QIcon::Normal);
    CapabilityButton* undoButton = new CapabilityButton(iconUndo, this);
    undoButton->setIconSize(QSize(32,32));
    undoButton->setFixedSize(32,32);
    connect(undoButton, &CapabilityButton::clicked, this, &Drawer::undoClicked);

    QIcon iconRedo;
    iconRedo.addFile(":/res/icons/redo.png",QSize(32,32),QIcon::Normal);
    CapabilityButton* redoButton = new CapabilityButton(iconRedo, this);
    redoButton->setIconSize(QSize(32,32));
    redoButton->setFixedSize(32,32);
    connect(redoButton, &CapabilityButton::clicked, this, &Drawer::redoClicked);

    QIcon iconCollapseAndExpand;
    iconCollapseAndExpand.addFile(":/res/icons/arraw_collapse.png",QSize(32,32),QIcon::Normal);
    iconCollapseAndExpand.addFile(":/res/icons/arraw_expand.png",QSize(32,32),QIcon::Selected);
    CapabilityButton* collapseAndExpandButton = new CapabilityButton(iconCollapseAndExpand, this);
    collapseAndExpandButton->setIconSize(QSize(32,32));
    collapseAndExpandButton->setFixedSize(32,32);
    collapseAndExpandButton->setCheckable(true);
    connect(collapseAndExpandButton, &CapabilityButton::clicked, this, [this](bool checked){
        checked ? collapse() : expand();
    });

    QIcon iconFreeze;
    iconFreeze.addFile(":/res/icons/freeze.png",QSize(32,32),QIcon::Normal);
    CapabilityButton* freezeButton = new CapabilityButton(iconFreeze, this);
    freezeButton->setIconSize(QSize(32,32));
    freezeButton->setFixedSize(32,32);
    freezeButton->setCheckable(true);
    connect(freezeButton, &CapabilityButton::clicked, this, &Drawer::freeze);

    QIcon iconDown;
    iconDown.addFile(":/res/icons/down.png",QSize(32,32),QIcon::Normal);
    CapabilityButton* downButton = new CapabilityButton(iconDown, this);
    downButton->setIconSize(QSize(32,32));
    downButton->setFixedSize(32,32);
    connect(downButton, &CapabilityButton::clicked, this, &Drawer::downClicked);

    QBoxLayout* layout = createLayout(Qt::Horizontal, 10, QMargins(10, 0, 10, 0));
    // layout->addStretch();
    layout->addWidget(undoButton);
    layout->addWidget(redoButton);
    layout->addWidget(collapseAndExpandButton);
    layout->addWidget(freezeButton);
    layout->addWidget(downButton);
    layout->addStretch();

    return layout;
}

QBoxLayout* Drawer::createLayout(Qt::Orientation o, int spacing, const QMargins& m)
{
    QBoxLayout* layout = new QBoxLayout(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    layout->setSpacing(spacing);
    layout->setContentsMargins(m);
    layout->setSizeConstraint(QBoxLayout::SetNoConstraint);
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
        foreachPen([c](Pen*pen){
            auto color = c;
            color.setAlpha(pen->color().alpha());
            pen->setColor(color);
        });
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

