#ifndef SETTINGVIEW_H
#define SETTINGVIEW_H

#include <QWidget>

namespace Ui {
class SettingView;
}

class SettingView : public QWidget
{
    Q_OBJECT

public:
    explicit SettingView(QWidget *parent = nullptr);
    ~SettingView();

private slots:
    void on_keySequenceEdit_Draw_keySequenceChanged(const QKeySequence &keySequence);

    void on_comboBox_language_currentTextChanged(const QString &text);

    void on_pushButton_openSettingDir_clicked();

    void on_pushButton_openDownloadDir_clicked();

    void on_pushButton_changeDownloadDir_clicked();

    void on_pushButton_resetsetting_clicked();

    void on_checkBox_saveBackground_clicked(bool checked);

private:
    Ui::SettingView *ui;
};

#endif // SETTINGVIEW_H
