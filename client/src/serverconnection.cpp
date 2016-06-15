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
    mSoc = new QSslSocket(this);
	mSoc->setProtocol(QSsl::SslProtocol::TlsV1_2OrLater);
	mSoc->setPeerVerifyMode(QSslSocket::PeerVerifyMode::VerifyPeer);

    connect(mSoc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(mSoc, SIGNAL(encrypted()), this, SLOT(connectionSuccess()));
    connect(mSoc, SIGNAL(encrypted()), this, SIGNAL(connectSuccess()));
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(dataFromServerReady()));
    connect(mSoc, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
	connect(mSoc, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sllErrors(const QList<QSslError> &)));
    mSoc->connectToHostEncrypted(mServerAddress, mPort);
}

void ServerConnection::sllErrors(const QList<QSslError> & errors) {
	if (errors.size() > 1) return;
	if (errors.first().error() != QSslError::HostNameMismatch) return;

	mSoc->ignoreSslErrors();
}

void ServerConnection::connectionError(QAbstractSocket::SocketError error)
{
    qDebug() << "Connection error(" << error << "): " << mSoc->errorString();
    QCoreApplication::exit(0);
}

void ServerConnection::dataFromServerReady()
{
    QByteArray a = mSoc->readAll();
//    qDebug() << "we've got data from server!";
    if(a.at(0) == 'm') {
        // qDebug() << "Message from server: " << a.mid(2);
        if(a.mid(2, 8) == "LOG_SUCC")
            emit loginSuccess(a.mid(11));
        else if(a.contains("LOG_FAIL") || a.contains("REG_FAIL"))
            emit fail();
        else if(a.contains("REG_SUCC"))
            emit registrationSuccess();
    }
    else if(!QJsonDocument::fromJson(a).isNull())
    {
        emit gotLoggedInPeers(a);
    }
}

