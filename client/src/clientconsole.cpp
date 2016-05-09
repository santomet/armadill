#include "clientconsole.h"

ClientConsole::ClientConsole(QStringList hostPort, QObject *parent) : QObject(parent),
    mServer(hostPort)
{
}

void ClientConsole::init()
{
    if(mServer.count() < 2)
    {
        qDebug() << "Incorrect usage. Please use -h for help";
        QCoreApplication::exit(1);
    }
    else
    {
        mbedtls_entropy_init(&mTLS_entropy); //start an entropy
        mbedtls_entropy_gather(&mTLS_entropy);

        mInputHelper = new UserInputHelper();
        connect(mInputHelper, SIGNAL(inputReady(QString)), this, SLOT(userInput(QString)));
        mMessages = new Messages(nullptr);
        //server
        mServerConnection = new ServerConnection(mServer.at(0), mServer.at(1).toInt());
        connect(mServerConnection, SIGNAL(connectSuccess()), this, SLOT(serverConnected()));
        connect(mServerConnection, SIGNAL(loginSuccess()), this, SLOT(loginSuccess()));
        connect(mServerConnection, SIGNAL(fail()), this, SLOT(fail()));
        connect(mServerConnection, SIGNAL(registrationSuccess()), this, SLOT(registrationSuccess()));
        connect(mServerConnection, SIGNAL(gotLoggedInPeers(QByteArray)), this, SLOT(loggedInPeersFromServer(QByteArray)));
        connect(this, SIGNAL(sendDataToServer(QByteArray)), mServerConnection, SLOT(sendDataToServer(QByteArray)));

        //peerServer
        startPeerServer();
        connect(mPeerServer, SIGNAL(newConnection(PeerConnection*)), this )

    }

}

void ClientConsole::loggedInPeersFromServer(QByteArray a)
{
    mMessages->parseJsonUsers(a, mOnlinePeerList);
    qDebug() << "logged in peers: " << mOnlinePeerList.count();
    foreach(peer p, mOnlinePeerList)
    {
        qDebug() << "nick: " << p.name << " addr: " << p.address << " port: " << p.listeningPort;
    }
    qDebug() << "write nickname to connect to peer";
}

void ClientConsole::connectToPeer(peer p)
{
    PeerConnection *peerConn = new PeerConnection(0, p, nullptr);
    int i = mPeerConnections.size();
    peerConn->setID(i);
    mPeerConnections(i, peerConn);
    Session *s = new Session(mNickName, peer.name, mTLS_entropy);
    mPeerSessions.insert(i, s);
    mNickConnectionMap.insert(peer.name, i);
}

void ClientConsole::connectionSuccessful(int id)
{
    mActivePeer = id;
    QByteArray identifyMessage;
    identifyMessage.append(Messages::armaSeparator); //only controller messages starts with separator
    identifyMessage.append("i"); //identification
    identifyMessage.append(Messages::armaSeparator);
    identifyMessage.append(mNickName);
    emit sendDataToPeer(identifyMessage);
    mExpectedInput = Message;
    qDebug() << "identification sent, type message";
    emit sendDataToPeer(Messages.addMessageHeader(mPeerSessions.value(id), QString(""), Messages::MsgType::PureDH));
}

void ClientConsole::newRemoteInitiatedConnection(PeerConnection *c)
{
    int i = mPeerConnections.size();
    mPeerConnections(i, c);
    c->setID(i);
}

void ClientConsole::dataFromPeerReady(int id, QByteArray a)
{
    if(a.at(0) == Messages::armaSeparator)
    {
        QList<QByteArray> identifyMessage = a.split(Messages::armaSeparator);
        if(identifyMessage.at(0) == "i")
        {
            mNickConnectionMap.insert(id, identifyMessage.at(1));
            mPeerSessions.insert(id, new Session(mNickName, mNickConnectionMap.value(id), mTLS_entropy));
            mExpectedInput = Message;
            qDebug() << "remote peer identified as: " << mNickConnectionMap.value(id);
        }
        else
            qDebug() << "Unknown identify message";
    }
    else
    {
        QString parsedMessage;
        mMessages->parseMessage(mPeerSessions.value(id), a, parsedMessage);
        qDebug() << parsedMessage;
    }
}



void ClientConsole::startPeerServer()
{
    mPeerServer = new ArmaTcpPeerServer(mMessages, mServerConnection);
    mPeerServer->listen(QHostAddress::Any, 0); //automatically selects the port
    mListeningPort = mPeerServer->serverPort();
    connect(mPeerServer, SIGNAL(newRemoteInitiatedConnection(PeerConnection*)), this, SLOT(newRemoteInitiatedConnection(PeerConnection*)));
    connect(mPeerServer, SIGNAL(endRemoteInitiatedConnection(PeerConnection*)), this, SLOT(endConnection(PeerConnection*)));
    connect(this, SIGNAL(connectionEstablished(PeerConnection*)), mPeerServer, SLOT(establishedConnection(PeerConnection*)));
}

void ClientConsole::userInput(QString Qline)
{
    switch (mExpectedInput)
    {
    case None :
        qDebug() << "Unexpected input!";
        break;
    case Idle :
        if(Qline == "p")
            emit sendDataToServer("j");
        else
        {
            peer peerToConnect = NULL;
            foreach(peer p, mOnlinePeerList)
            {
                if(Qline == p.name)
                    peerToConnect = p;
            }
            if(peerToConnect == NULL)
            {
                qDebug() << "selected peer not found";
            }
            else
                connectToPeer(peerToConnect);
        }
        break;
    case Message:
        emit sendDataToPeer(Messages.createRegularMessage(mPeerSessions.value(mActivePeer), Qline));
        break;
    case LoginOrRegister :
        if(Qline == "l")
        {
            mExpectedInput = NickNameLogin;
            qDebug() << "Nick:";
        }
        else if(Qline == "r")
        {
            mExpectedInput = NickNameRegister;
            qDebug() << "select nickname:";
        }
        else
            qDebug() << "WRONG ANSWER!";
        break;
    case NickNameLogin :
        mNickName = Qline;
        qDebug() << "Password:";
        mExpectedInput = PasswordLogin;
        break;
    case NickNameRegister :
        mNickName = Qline;
        qDebug() << "plese select password:";
        mExpectedInput = PasswordRegister;
        break;
    case PasswordLogin :
        mPassword = Qline;
        qDebug() << mNickName << ":" << mPassword;
        emit sendDataToServer(mMessages->createLoginMessage(mNickName, mPassword, mListeningPort, false));
        mExpectedInput = None;
        break;
    case PasswordRegister :
        mPassword = Qline;
        qDebug() << mNickName << ":" << mPassword;
        emit sendDataToServer(mMessages->createLoginMessage(mNickName, mPassword, true));
        mExpectedInput = None;
        break;
    default :
        break;
    }
}

