#include "config.h"
#include "dbapplication.h"
#include "settingview.h"
#include "ui_settingview.h"

#include <QDesktopServices>
#include <QFileDialog>

SettingView::SettingView(QWidget *parent)
    :QWidget(parent, Qt::WindowStaysOnTopHint | Qt::Dialog)
    ,ui(new Ui::SettingView)
{
    ui->setupUi(this);

    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::INTERNAL);
    Q_ASSERT(handle);
    ui->settingPathEdit->setText(handle->getString("const.path.setting"));
    ui->downloadDirEdit->setText(handle->getString("dir.download"));
    ui->keySequenceEdit_Draw->setKeySequence(QKeySequence::fromString(handle->getString("key.global.draw")));
    ui->comboBox_language->setCurrentText(handle->getString("language"));

    connect(config, &Config::configChanged, this, [this, handle](Config::ChangedType type, const QString &id){
        Q_UNUSED(type);
        if(id == "dir.download")
            ui->downloadDirEdit->setText(handle->getString("dir.download"));
    });
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

void SettingView::on_comboBox_language_currentTextChanged(const QString &text)
{
    // qDebug() << "lan" << text;
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::USER);
    Q_ASSERT(handle);
    handle->setValue("language", text);
}

void SettingView::on_pushButton_openSettingDir_clicked()
{
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::USER);
    Q_ASSERT(handle);
    QDesktopServices::openUrl(QUrl::fromLocalFile(handle->getString("const.dir.setting")));
}


void SettingView::on_pushButton_openDownloadDir_clicked()
{
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::USER);
    Q_ASSERT(handle);
    QDesktopServices::openUrl(QUrl::fromLocalFile(handle->getString("dir.download")));
}


void SettingView::on_pushButton_changeDownloadDir_clicked()
{
    Config* config = static_cast<DBApplication*>(qApp)->getSingleton<Config>();
    ConfigHandle* handle = config->getConfigHandle(Config::USER);
    Q_ASSERT(handle);

    QString downloadDir = handle->getString("dir.download");

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),downloadDir,QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);

    handle->setValue("dir.download", dir);
}

