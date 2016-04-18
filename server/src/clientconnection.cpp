#include "clientconnection.h"

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

ClientConnection::~ClientConnection()
{
    this->disconnect();
    this->mSoc->disconnect();
    if(mThread!=nullptr)
    {
        mThread->quit();
        if(!mThread->wait(1000))
            mThread->terminate();
    }
    delete mSoc;
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
    connect(mSoc, SIGNAL(readyRead()), this, SLOT(readDataFromClient()));
    qDebug() << "connection created";
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
    QString nickname, password;
    bool reg;
    int count = message.count('#');
    if(count!=2)
        return false;
    QList<QByteArray> list = message.split('#');
    if(list.at(0) == "l")
        reg = false;
    else if(list.at(0) == "r")
        reg = true;
    else
        return false;

    nickname = QString::fromUtf8(list.at(1));
    password = QString::fromUtf8(QByteArray::fromBase64(list.at(2)));
    if(reg)
    {
        if(!mServerManager->newRegistration(nickname, password))
        {
            this->sendDataToClient("m#REG_FAIL");
        }
        else
        {
            this->sendDataToClient("m#REG_SUC");
        }
    }
    else
    {
        if(!mServerManager->login(nickname, password, mSoc->peerAddress().toString(), mSoc->peerPort(), "NO_CERT"))
        {
            this->sendDataToClient("m#LOG_FAILED");
        }
        else
        {
            this->sendDataToClient("m#LOG_SUC");
            mNickName = nickname;
        }
    }


    return true;
}

