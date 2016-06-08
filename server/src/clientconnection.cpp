#include "clientconnection.h"
#include "crypto.h"

CertificateManager certMngr;

ClientConnection::ClientConnection(qintptr socDescriptor, ServerManager *ser, bool newThread, QObject *parent) : QObject(parent),
    mServerManager(ser),
    mSocDescriptor(socDescriptor)
{
    if(newThread)
    {
        mThread = new QThread(this);
        this->moveToThread(mThread);
        connect(mThread, SIGNAL(started()), this, SLOT(init()));
        mThread->start();
    }
    else
    {
        this->init();
    }
}

void ClientConnection::destroy()
{
	delete mSoc;
}

ClientConnection::~ClientConnection()
{
    this->disconnect();
    this->mSoc->disconnect();
	QMetaObject::invokeMethod(this, "destroy", Qt::BlockingQueuedConnection);
    if(mThread!=nullptr)
    {
        mThread->quit();
        if(!mThread->wait(1000))
            mThread->terminate();
    }
    qDebug() << "disconnected" << mPeerAddress;
    //TODO
//    if(!mNickName.isEmpty())
//        mServerManager->logout(mNickName)
}

void ClientConnection::init()
{
    mSoc = new QTcpSocket(this);
    mSoc->setSocketDescriptor(mSocDescriptor);
    if(mThread != nullptr)
    {
        connect(mSoc, SIGNAL(disconnected()), this, SLOT(doneSlot()));
    }
    else
    {
        connect(mSoc, SIGNAL(disconnected()), this, SLOT(deleteLater()));

    }
    mPeerAddress = mSoc->peerAddress().toString();
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(readDataFromClient()));
    qDebug() << "connection created: " << mPeerAddress;
}

void ClientConnection::readDataFromClient()
{
    QByteArray a = mSoc->readAll();
    if(a.at(0) == 'l' || a.at(0) == 'r')
    {
        parseLoginMessage(a);
    }
    else if(a.at(0) == 'j')
    {
        sendDataToClient(mServerManager->JsonToByteArray(mServerManager->exportOnlineUsersJson()));
    }
}


bool ClientConnection::parseLoginMessage(QByteArray& message) {
    bool reg;
    int count = message.count('#');
    if(count < 3) return false;

    QList<QByteArray> list = message.split('#');
    if(list.at(0) == "l")
        reg = false;
    else if(list.at(0) == "r")
        reg = true;
    else
        return false;

    //TODO certificate
    QString nickname = QString::fromUtf8(QByteArray::fromBase64(list.at(1)));
    QString password = QString::fromUtf8(QByteArray::fromBase64(list.at(2)));
	int port = QString::fromUtf8(QByteArray::fromBase64(list.at(3))).toInt();

    if(reg) {
        if (count != 3) return false;
        if(!mServerManager->newRegistration(nickname, password)) {
            this->sendDataToClient("m#REG_FAIL");
        }
        else
        {
            this->sendDataToClient("m#REG_SUCC");
        }
    }
    else {
		if (count != 4) return false;

        if(!mServerManager->login(nickname, password, mSoc->peerAddress().toString(), port, "NO_CERT")) {
            this->sendDataToClient("m#LOG_FAILED");
        }
        else {
			QByteArray cert, req = list.at(4);
			try {
				certMngr.createCert(nickname, req, cert);
				QByteArray resp = "m#LOG_SUCC#";
				resp.append(cert);
				this->sendDataToClient(resp);
				mNickName = nickname;
			}
			catch (CryptoException & e) {
				qDebug() << "Login Failed: " << e.what() << " (" << e.error() << ")";
				this->sendDataToClient("m#LOG_FAILED");
			}
        }
    }
    return true;
}

