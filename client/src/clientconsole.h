#ifndef CLIENTCONSOLE_H
#define CLIENTCONSOLE_H

#include <QDebug>
#include <QCommandLineParser>
#include <QObject>

class ClientConsole : public QObject
{
    Q_OBJECT
public:
    explicit ClientConsole(QCommandLineParser *parser, QObject *parent = 0);

signals:

public slots:
};

#endif // CLIENTCONSOLE_H
