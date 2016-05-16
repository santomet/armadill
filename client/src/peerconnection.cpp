#include "peerconnection.h"

PeerConnection::PeerConnection(qintptr soc, peer _peer, ServerConnection *server, QObject *parent)
    : QObject(parent), mPeer(_peer), mServer(server), mSocDescriptor(soc)
{
    if(soc == 0 && mPeer.name.isEmpty()) {
		//TODO: throw exception or something, this is useless
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
	//TODO: add my certificate
	//mSoc->setLocalCertificate(localCert);
	//mSoc->setPrivateKey(privateKey);

    connect(mSoc, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    connect(mSoc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(dataFromPeerReady()));
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(mSoc, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sllErrorsClient(const QList<QSslError> &)));

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

void PeerConnection::sllErrorsClient(const QList<QSslError> & errors) {
	if (errors.size() > 1) return;
	if (errors.first().error() != QSslError::HostNameMismatch) return;

	//QStringList name = errors.first().certificate().subjectInfo(QSslCertificate::SubjectInfo::CommonName);
	//if (name.size() != 1) return;

	emit mSoc->ignoreSslErrors();
}

void PeerConnection::sendDataToPeer(QByteArray a)
{
    mSoc->write(a);
}


void PeerConnection::connectionError(QAbstractSocket::SocketError error) {
    qDebug() << "Connection error(" << error << "): " << mSoc->errorString();
    QCoreApplication::exit(0);
}

