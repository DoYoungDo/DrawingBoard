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
}

SettingView::~SettingView()
{
    delete ui;
}
