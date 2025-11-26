#include "preview.h"
#include "tools.h"
#include "config.h"
#include "dbapplication.h"

#include <capabilitybutton.h>

#include <QBoxLayout>
#include <QDateTime>
#include <QImageWriter>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

Preview::Preview(const QPixmap& pix, QWidget* parent)
    : QWidget{parent}
{
    this->pix = pix;

    DBApplication* app = static_cast<DBApplication*>(qApp);
    ConfigHandle* handle = app->getSingleton<Config>()->getConfigHandle(Config::INTERNAL);
    localFilePath = handle->getString("dir.download")
            + "/"
            + app->applicationName() +  "-" + QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()).toString("YYYY-MM-DD-hh-mm-ss") + ".png";

    setupUi();
}

void Preview::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setPen(Qt::transparent);
    p.setBrush(Qt::transparent);

    p.save();
    p.setBrush(Qt::gray);
    p.drawRoundedRect(this->rect(),5,5);
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
    for(auto btn : std::as_const(btns))
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

void Preview::download()
{
    // pix.copy(pix.rect()).save(localFilePath);
    QImage img = pix.toImage();
    QImageWriter w(localFilePath);
    w.write(img);
    // QFile file(localFilePath);
    // if(file.open(QIODevice::WriteOnly))
    // {
    //     pix.save(&file);

    //     file.close();
    // }
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
    CapabilityButton* minButton = new CapabilityButton(QIcon(":/res/icons/arrow-Down.png"), this);
    minButton->setIconSize(QSize(32,32));
    minButton->setFixedSize(QSize(32,32));
    connect(minButton, &CapabilityButton::clicked, this, &Preview::minButtonClicked);
    CapabilityButton* closeButton = new CapabilityButton(QIcon(":/res/icons/close.png"), this);
    closeButton->setIconSize(QSize(32,32));
    closeButton->setFixedSize(QSize(32,32));
    connect(closeButton, &CapabilityButton::clicked, this, &Preview::closeButtonClicked);
    QBoxLayout* layout = TOOLS::createLayout(Qt::Horizontal, 10, QMargins(0, 10, 0, 0));
    layout->addStretch();
    layout->addWidget(minButton);
    layout->addWidget(closeButton);
    layout->addStretch();
    return layout;
}
