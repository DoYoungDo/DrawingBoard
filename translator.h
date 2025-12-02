#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QMap>
#include <QTranslator>

class Translator : public QTranslator
{
    Q_OBJECT
public:
    Translator(const QString& lanName = QString(), QObject *parent = nullptr);

    // QTranslator interface
public:
    virtual QString translate(const char *context, const char *sourceText, const char *disambiguation, int n) const override;
    virtual bool isEmpty() const override;

private:
    QMap<QString, QString> words;
};

#endif // TRANSLATOR_H
