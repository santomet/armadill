#include "serverconnection.h"
#include "messages.h"

ServerConnection::ServerConnection(QString host, int port, QObject *parent) : QObject(parent),
    mServerAddress(host),
    mPort(port)
{
    mThread = new QThread();
    this->moveToThread(mThread);
    connect(mThread, SIGNAL(started()), this, SLOT(init()));
    mThread->start();
}

void ServerConnection::sendDataToServer(QByteArray array)
{
    mSoc->write(array);
}

void ServerConnection::init()
{
    mSoc = new QTcpSocket(this);
    connect(mSoc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(mSoc, SIGNAL(connected()), this, SLOT(connectionSuccess()));
    connect(mSoc, SIGNAL(connected()), this, SIGNAL(connectSuccess()));
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(dataFromServerReady()));
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
    mSoc->connectToHost(mServerAddress, mPort);
}

void ServerConnection::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "Connection error: " << mSoc->errorString();
    QCoreApplication::exit(0);
}

void ServerConnection::dataFromServerReady()
{
    QByteArray a = mSoc->readAll();
    qDebug() << "we've got data from server!";
    QList<QByteArray> list = a.split('#');
    //TODO DEFINE
}

