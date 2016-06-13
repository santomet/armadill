#include "peerconnection.h"
#include <iostream>

PeerConnection::PeerConnection(qintptr soc, peer _peer, ServerConnection *server, QObject *parent)
    : QObject(parent), mPeer(_peer), mServer(server), mSocDescriptor(soc)
{
    if(soc == 0 && mPeer.name.isEmpty()) {
        qDebug() << "[ERROR] Creating a connection to peer: cannot call empty constructor!";
        return;
    }

    mThread = new QThread(this);
    this->moveToThread(mThread);
    connect(mThread, SIGNAL(started()), this, SLOT(init()));
    mThread->start();

}

void PeerConnection::init() {
    mSoc = new QSslSocket(this);
	mSoc->setProtocol(QSsl::SslProtocol::TlsV1_2OrLater);
    mSoc->setPeerVerifyMode(QSslSocket::PeerVerifyMode::VerifyPeer);
    mSoc->setLocalCertificate(Messages::localCert);
    mSoc->setPrivateKey(Messages::localKey);

    connect(mSoc, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    connect(mSoc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mSoc, SIGNAL(encrypted()), this, SLOT(successfulEncryptedConnection()));
    connect(mSoc, SIGNAL(encrypted()), this, SLOT(connectionSuccess()));
    connect(mSoc, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sllErrorsClient(const QList<QSslError> &)));

    if(mSocDescriptor != 0) {
        mSoc->setSocketDescriptor(mSocDescriptor);
        mSoc->startServerEncryption();
    }
    else {
        mSoc->connectToHostEncrypted(mPeer.address, mPeer.listeningPort, mPeer.name);
    }
    mPeerAddress = mSoc->peerAddress().toString();
    mPeerPort = mSoc->peerPort();
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(readDataFromClient()));
    qDebug() << "connection created: " << mPeerAddress;
}

void PeerConnection::sllErrorsClient(const QList<QSslError> & errors) {
//	if (errors.size() > 1) return;
//	if (errors.first().error() != QSslError::HostNameMismatch) return;

    //TODO: Tuna by sa zislo len zistit pre nas potrebne veci: Ci je to podpisane nasim certifikatom
    //(vid TODO hore) a ci to ma tych 24 hodin max
    qDebug() << "ERRORS:" << errors;
    mSoc->ignoreSslErrors();
}

void PeerConnection::successfulEncryptedConnection() { 
	mPeer.name = mSoc->peerCertificate().subjectInfo(QSslCertificate::CommonName).at(0);
}

void PeerConnection::sendDataToPeer(QByteArray a)
{
    mSoc->write(a);
}


void PeerConnection::connectionError(QAbstractSocket::SocketError error) {
    qDebug() << "Connection error(" << error << "): " << mSoc->errorString();
    QCoreApplication::exit(0);
}

