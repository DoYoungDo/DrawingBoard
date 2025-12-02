#include "config.h"
#include "dbapplication.h"
#include "settingview.h"
#include "ui_settingview.h"

SettingView::SettingView(QWidget *parent)
    :QWidget(parent, Qt::WindowStaysOnTopHint)
    ,ui(new Ui::SettingView)
{
    ui->setupUi(this);

    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);
    ui->settingPathEdit->setText(handle->getString("path.setting"));
    ui->downloadDirEdit->setText(handle->getString("dir.download"));
    ui->keySequenceEdit_Draw->setKeySequence(QKeySequence::fromString(handle->getString("key.global.draw")));
}

SettingView::~SettingView()
{
    delete ui;
}

void SettingView::on_keySequenceEdit_Draw_keySequenceChanged(const QKeySequence &keySequence)
{
    // qDebug() << "key" << keySequence;
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::USER);
    Q_ASSERT(handle);
    handle->setValue("key.global.draw", keySequence.toString());
}

