#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include <QDebug>
#include <QCommandLineParser>
#include <QObject>

class ServerConsole : public QObject
{
    friend class UTest;
    Q_OBJECT
public:
    explicit ServerConsole(QCommandLineParser *parser, QObject *parent = 0);
    bool trueReturningTestMethod() {return true;}
    bool falseReturningTestMethod() {return false;}

signals:

public slots:
};

#endif // SERVERCONSOLE_H
