#ifndef CLIENTCONSOLE_H
#define CLIENTCONSOLE_H

#include <QDebug>
#include <QCommandLineParser>
#include <QObject>
#include "messages.h"
#include "serverconnection.h"

class ClientConsole : public QObject
{
    friend class UTest;
    Q_OBJECT
public:
    explicit ClientConsole(QObject *parent = 0);


private:
//---------tests
    bool trueReturningTestMethod() {return true;}
    bool falseReturningTestMethod() {return false;}

    QCommandLineParser *mParser;

    QQueue<Messages::ArmaMessage*> messageQueue; //max 10



signals:

public slots:
    /*!
     * \brief init
     *  initializes the client
     * \return
     */
    void init() {}

};

#endif // CLIENTCONSOLE_H
