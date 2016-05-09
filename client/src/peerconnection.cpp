#include "peerconnection.h"

PeerConnection::PeerConnection(qintptr soc, peer _peer, ServerConnection *server, QObject *parent)
    : QObject(parent), mPeer(_peer), mServer(server), mSocDescriptor(soc)
{
    if(soc == 0 && mPeer.name.isEmpty())
    {
        qDebug() << "[ERROR] Creating a connection to peer: cannot call empty constructor!";
        return;
    }

    mThread = new QThread(this);
    this->moveToThread(mThread);
    connect(mThread, SIGNAL(started()), this, SLOT(init()));
    mThread->start();

}

void PeerConnection::init()
{
    mSoc = new QTcpSocket(this);
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    connect(mSoc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(dataFromPeerReady()));
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(disconnected()));
    if(mSocDescriptor != 0)
    {
        mSoc->setSocketDescriptor(mSocDescriptor);
    }
    else
    {
        connect(mSoc, SIGNAL(connected()), this, SLOT(connectionSuccess()));
        mSoc->connectToHost(mPeer.address, mPeer.listeningPort);
    }
    mPeerAddress = mSoc->peerAddress().toString();
    mPeerPort = mSoc->peerPort();
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(readDataFromClient()));
    qDebug() << "connection created: " << mPeerAddress;
}

void PeerConnection::sendDataToPeer(QByteArray a)
{
    mSoc->write(a);
}


void PeerConnection::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "Connection error: " << mSoc->errorString();
    QCoreApplication::exit(0);
}

