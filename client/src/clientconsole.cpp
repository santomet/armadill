#include "clientconsole.h"

#define MBEDTLS_PEM_PARSE_C

#include "../../include/mbedtls/x509_crt.h"

ClientConsole * clientConsole;

ClientConsole::ClientConsole(QStringList hostPort, QTextStream & out, QObject *parent) : QObject(parent), mServer(hostPort), Out(out) {
	clientConsole = this;
}

void ClientConsole::init()
{
    if(mServer.count() < 2) {
        Out << "Incorrect usage. Please use -h for help" << endl;
        QCoreApplication::exit(1);
    }
    else {
        mbedtls_entropy_init(&mTLS_entropy); //start an entropy
        mbedtls_entropy_gather(&mTLS_entropy);

        QFile cert("cert.crt");
		cert.open(QIODevice::ReadOnly);
		QSslSocket::addDefaultCaCertificate(QSslCertificate(cert.readAll()));

        mInputHelper = new UserInputHelper();
        connect(mInputHelper, SIGNAL(inputReady(QString)), this, SLOT(userInput(QString)));
        mMessages = new Messages(nullptr);
        //server
        mServerConnection = new ServerConnection(mServer.at(0), mServer.at(1).toInt());
        connect(mServerConnection, SIGNAL(connectSuccess()), this, SLOT(serverConnected()));
        connect(mServerConnection, SIGNAL(loginSuccess(QByteArray)), this, SLOT(loginSuccess(QByteArray)));
        connect(mServerConnection, SIGNAL(fail()), this, SLOT(fail()));
        connect(mServerConnection, SIGNAL(registrationSuccess()), this, SLOT(registrationSuccess()));
        connect(mServerConnection, SIGNAL(gotLoggedInPeers(QByteArray)), this, SLOT(loggedInPeersFromServer(QByteArray)));
        connect(this, SIGNAL(sendDataToServer(QByteArray)), mServerConnection, SLOT(sendDataToServer(QByteArray)));

        //peerServer
        startPeerServer();
//        connect(mPeerServer, SIGNAL(newConnection(PeerConnection*)), this );

    }

}

void ClientConsole::loginSuccess(QByteArray cert) {
	Out << "You can load peers from server (p)" << endl; 
	mExpectedInput = Idle;

	cert.append('\0');

	Messages::localCert = QSslCertificate(cert);
}

void ClientConsole::loggedInPeersFromServer(QByteArray a)
{
    mMessages->parseJsonUsers(a, mOnlinePeerList);
    Out << "logged in peers: " << mOnlinePeerList.count() << endl;
    foreach(peer p, mOnlinePeerList) {
		Out << "nick: " << p.name << endl;
		// qDebug() << "nick: " << p.name << " addr: " << p.address << " port: " << p.listeningPort;
    }
    Out << "write nickname to connect to peer" << endl;
}


static void sendingHelperFunc(PeerConnection * ptr, const QByteArray & data) {
	ptr->sendDataToPeer(data);
}

void ClientConsole::connectToPeer(peer p)
{
    PeerConnection *peerConn = new PeerConnection(true, 0, p, nullptr);
    connect(peerConn, SIGNAL(peerConnected(int)), this, SLOT(connectionSuccessful(int)));
    connect(peerConn, SIGNAL(dataReady(int,QByteArray)), this, SLOT(dataFromPeerReady(int,QByteArray)));
    connect(peerConn, SIGNAL(peerDisconnected(int)), this, SLOT(endConnection(int)));
	int i = currentID++;
    peerConn->setID(i);
    mPeerConnections.insert(i, peerConn);

}

void ClientConsole::connectionSuccessful(int id)
{
    PeerConnection *c = mPeerConnections.value(id);
    Session *s = new Session(mNickName, c->getPeer().name, &mTLS_entropy, c->isInitiator());
    s->setSender(std::bind(sendingHelperFunc, c, std::placeholders::_1));
    mPeerSessions.insert(id, s);
    mNickConnectionMap.insert(c->getPeer().name, id);

    mActivePeer = id;
    // qDebug() << "identification sent, type message";
    sendDataToPeer(mMessages->addMessageHeader(*(mPeerSessions.value(id)), QString("").toUtf8(), Messages::MsgType::PureDH, Messages::MsgType::PureDH));
    this->mExpectedInput = ExpectedUserInput::Message;
}

void ClientConsole::newRemoteInitiatedConnection(PeerConnection *c)
{
    connect(c, SIGNAL(dataReady(int,QByteArray)), this, SLOT(dataFromPeerReady(int,QByteArray)));
    connect(c, SIGNAL(peerDisconnected(int)), this, SLOT(endConnection(int)));
    int i = currentID++;
    mPeerConnections.insert(i, c);
    c->setID(i);
    connect(c, SIGNAL(peerConnected(int)), this, SLOT(connectionSuccessful(int)));
}

void ClientConsole::endConnection(int id)
{
    qDebug() << "Disconnected peer: " << mNickConnectionMap.key(id);
    mNickConnectionMap.remove(mNickConnectionMap.key(id));
    this->mExpectedInput = Idle;
    qDebug() << "please select another peer to connect (or (p) to renew online peers)";
}

Session & sessionHandler(ClientConsole & c, const QString & nick) {
	auto it = c.mNickConnectionMap.find(nick);
	int id = *it;
	auto it2 = c.mPeerSessions.find(id);
	return **it2;
}

void ClientConsole::dataFromPeerReady(int id, QByteArray a)
{
    QString parsedMessage(QString::fromUtf8(a));
    Messages::parseMessage(std::bind(sessionHandler, std::ref(*this), std::placeholders::_1), a, [=](Session & session, Messages::MsgType type, const Messages::ReceivedMessage & payload) { this->callbackHandler(session, type, payload); });
    // qDebug() << "DATA supposed nickname(from connection) : " << mNickConnectionMap.key(id) << "MESSAGE: " << parsedMessage;
}

void ClientConsole::callbackHandler(Session & session, Messages::MsgType type, const Messages::ReceivedMessage & payload) {
	switch (type) {
	case Messages::RegularMessage:
	case Messages::RegularMessageDH:
		Out << session.getPartnerName() << ": " << payload.messageText << endl;
		break;
	case Messages::FileMessage:
	case Messages::FileMessageDH:
		Messages::FileReceivingContext::receiveChunk(session, payload.messageText);
		break;
	case Messages::FileResponse:
	case Messages::FileResponseDH:
		Messages::FileSendingContext::confirmFile(session, payload.messageText);
		break;
	case Messages::FileContext:
	case Messages::FileContextDH:
		Messages::FileReceivingContext::receiveFile(session, payload.messageText);
		break;
	}
};


void ClientConsole::startPeerServer()
{
    mPeerServer = new ArmaTcpPeerServer(mServerConnection);
    mPeerServer->listen(QHostAddress::Any, 0); //automatically selects the port
    mListeningPort = mPeerServer->serverPort();
    connect(mPeerServer, SIGNAL(newRemoteInitiatedConnection(PeerConnection*)), this, SLOT(newRemoteInitiatedConnection(PeerConnection*)));
    connect(mPeerServer, SIGNAL(endRemoteInitiatedConnection(PeerConnection*)), this, SLOT(endConnection(PeerConnection*)));
    connect(this, SIGNAL(connectionEstablished(PeerConnection*)), mPeerServer, SLOT(establishedConnection(PeerConnection*)));
}

void ClientConsole::userInput(QString Qline) {
	if (Qline.size() < 1) return;
	if (Qline.at(0) == '/') {
		ClientConsole::command(Qline.mid(1));
		return;
	}
    switch (mExpectedInput)
    {
    case None :
       Out << "Wait please!" << endl;
        break;
    case Idle :
        if(Qline == "p")
            emit sendDataToServer("j");
		else
		{
			peer peerToConnect = {};
			foreach(peer p, mOnlinePeerList)
			{
				if (Qline == p.name)
					peerToConnect = p;
			}
			if (peerToConnect.name.isEmpty())
			{
				Out << "Selected peer not found. Please try again:" << endl;
			}
			else {
				connectToPeer(peerToConnect);
				mExpectedInput = None;
			}
		}
        break;
    case Message:
        sendDataToPeer(mMessages->createRegularMessage(*mPeerSessions.value(mActivePeer), Qline));
        break;
    case LoginOrRegister :
        if(Qline == "l")
        {
            mExpectedInput = NickNameLogin;
            Out << "Nick:" << endl;
        }
        else if(Qline == "r")
        {
            mExpectedInput = NickNameRegister;
            Out << "Choose nickname: " << endl;
        }
        else
            Out << "Unknown command!" << endl;
        break;
    case NickNameLogin :
        mNickName = Qline;
        Out << "Password:" << endl;
        mExpectedInput = PasswordLogin;
        break;
    case NickNameRegister :
        mNickName = Qline;
        Out << "Plese select password:" << endl;
        mExpectedInput = PasswordRegister;
        break;
    case PasswordLogin :
        mPassword = Qline;
        // qDebug() << mNickName << ":" << mPassword;
        emit sendDataToServer(mMessages->createLoginMessage(mNickName, mPassword, mListeningPort, false));
        mExpectedInput = None;
        break;
    case PasswordRegister :
        mPassword = Qline;
        // qDebug() << mNickName << ":" << mPassword;
        emit sendDataToServer(mMessages->createLoginMessage(mNickName, mPassword, mListeningPort, true));
        mExpectedInput = None;
        break;
    default :
        break;
    }
}

void ClientConsole::sendDataToPeer(QByteArray a)
{
    PeerConnection *p = this->mPeerConnections.value(mActivePeer);
    p->sendDataToPeer(a);
}

void ClientConsole::command(QString cmdtext) {
	if (cmdtext == "exit") {
		QCoreApplication::exit(0);
	}
	else if (cmdtext.startsWith("file")) {
        //Messages::FileSendingContext::sendFile(*mPeerSessions.value(mActivePeer), cmdtext.mid(5), mPeerSessions.value(mActivePeer)->send);
	}
    else if (cmdtext == "quit") {
        if(mActivePeer == -1)
        {
            qDebug() << "you are not connected to any peer";
            return;
        }
        QMetaObject::invokeMethod(mPeerConnections.value(mActivePeer), "disconnectPeer");
        mActivePeer = -1;
    }
}
