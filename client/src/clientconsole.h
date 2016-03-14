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
    explicit ClientConsole(QCommandLineParser *parser, QObject *parent = 0);

    /*!
     * \brief init
     *  initializes the client
     * \return
     */
    bool init();


private:
//---------tests
    bool trueReturningTestMethod() {return true;}
    bool falseReturningTestMethod() {return false;}



    QQueue<Messages::ArmaMessage*> messageQueue; //max 10


signals:

public slots:
};

#endif // CLIENTCONSOLE_H
