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

private:
    Ui::SettingView *ui;
};

#endif // SETTINGVIEW_H
