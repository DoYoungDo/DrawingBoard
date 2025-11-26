#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>

class QBoxLayout;

class Preview : public QWidget
{
    Q_OBJECT
public:
    explicit Preview(const QPixmap& pix, QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

signals:
    void doubleClicked();
    void minButtonClicked();
    void closeButtonClicked();
public:
    void setMinModeSize(const QSize& s);
    QSize getMinModeSize();
    void setMaxModeSize(const QSize& s);
    QSize getMaxModeSize();

    void showMin();
    void showMax();
    bool isMaxMode();
    bool isMaxMode(const QSize& s);

public slots:
    void download();

private:
    void setupUi();
    QBoxLayout* setupToolButtonUi();

private:
    QPixmap pix;
    QSize maxSize;
    QSize minSize;
    QString localFilePath;
};

#endif // PREVIEW_H
