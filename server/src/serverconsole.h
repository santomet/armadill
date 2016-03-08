#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include <QDebug>
#include <QCommandLineParser>
#include <QObject>

class ServerConsole : public QObject
{
    Q_OBJECT
public:
    explicit ServerConsole(QCommandLineParser *parser, QObject *parent = 0);

signals:

public slots:
};

#endif // SERVERCONSOLE_H
