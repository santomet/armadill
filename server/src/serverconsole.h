#ifndef SERVERCONSOLE_H
#define SERVERCONSOLE_H

#include <QDebug>
#include <QCommandLineParser>
#include <QObject>
#include <QtNetwork>
#include "servermanager.h"
#include "clientconnection.h"

class ArmaTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ArmaTcpServer(ServerManager *servermanager, QObject *parent) : QTcpServer(parent), mServerManager(servermanager)
    {}


    QList<ClientConnection*> mConnections;

public slots:
    void deleteConnection(ClientConnection *c) {mConnections.removeOne(c); delete c;}
protected:
    void incomingConnection(qintptr handle) override {ClientConnection *c = new ClientConnection(handle, mServerManager, true);
                                                     mConnections.append(c);
                                                     connect(c, SIGNAL(done(ClientConnection*)), this, SLOT(deleteConnection(ClientConnection*)), Qt::QueuedConnection);
                                                     }
    ServerManager *mServerManager;
};

class ServerConsole : public QObject
{
    friend class UTest;
    Q_OBJECT
public:
    explicit ServerConsole(QCommandLineParser *arguments, QObject *parent = 0);

signals:

public slots:

protected slots:
    void init();

private:
    ServerManager *mServerManager;
    ArmaTcpServer *mTcpServer;
    int mPort;
    QCommandLineParser *mArguments;
};

#endif // SERVERCONSOLE_H
