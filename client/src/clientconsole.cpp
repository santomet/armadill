#include "clientconsole.h"

#define MBEDTLS_PEM_PARSE_C

#include "../../include/mbedtls/x509_crt.h"

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
		std::cin.get();
        mbedtls_entropy_init(&mTLS_entropy); //start an entropy
        mbedtls_entropy_gather(&mTLS_entropy);

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

		QFile cert("test/ARMADILL.crt");
		cert.open(QIODevice::ReadOnly);
		QSslSocket::addDefaultCaCertificate(QSslCertificate(cert.readAll()));

        //peerServer
        startPeerServer();
//        connect(mPeerServer, SIGNAL(newConnection(PeerConnection*)), this );

    }

}

void ClientConsole::loginSuccess(QByteArray cert) {
	qDebug() << "You can load peers from server (p)"; 
	mExpectedInput = Idle;

	cert.append('\0');
	qDebug() << cert;

	char info[4096];
	mbedtls_x509_crt mcrt;
	mbedtls_x509_crt_init(&mcrt);
	mbedtls_x509_crt_parse(&mcrt, reinterpret_cast<uchar *>(cert.data()), cert.length()+1);
	mbedtls_x509_crt_info(info, 4000, "", &mcrt);
	std::cout << info << endl;
	mbedtls_x509_crt_free(&mcrt);

	Messages::localCert = QSslCertificate(cert);
	qDebug() << Messages::localCert;
	qDebug() << Messages::localCert.subjectInfo(QSslCertificate::SubjectInfo::CommonName);
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


static void sendingHelperFunc(PeerConnection * ptr, const QByteArray & data) {
	ptr->sendDataToPeer(data);
}

void ClientConsole::connectToPeer(peer p)
{
    PeerConnection *peerConn = new PeerConnection(0, p, nullptr);
    connect(peerConn, SIGNAL(peerConnected(int)), this, SLOT(connectionSuccessful(int)));
    connect(peerConn, SIGNAL(dataReady(int,QByteArray)), this, SLOT(dataFromPeerReady(int,QByteArray)));
	int i = currentID++;
    peerConn->setID(i);
    mPeerConnections.insert(i, peerConn);

}

void ClientConsole::connectionSuccessful(int id)
{
    PeerConnection *c = mPeerConnections.value(id);
    Session *s = new Session(mNickName, c->getPeer().name, &mTLS_entropy);
    s->setSender(std::bind(sendingHelperFunc, c, std::placeholders::_1));
    mPeerSessions.insert(id, s);
    mNickConnectionMap.insert(c->getPeer().name, id);

    //TODO tuna by som asi medzi nimi len poslal prazdnu spravu. Ale uz nepamatam, ty lepsie poznas jak spravy funguju
    //Poznamka: sendDataToPeer() odosiela vzdy peerovi, ktoreho id je v mActivePeer, to sa teraz pouziva aj pri obycajnych spravach
    mActivePeer = id;
    qDebug() << "identification sent, type message";
    sendDataToPeer(mMessages->addMessageHeader(*(mPeerSessions.value(id)), QString("").toUtf8(), Messages::MsgType::PureDH, Messages::MsgType::PureDH));
    this->mExpectedInput = ExpectedUserInput::Message;
}

void ClientConsole::newRemoteInitiatedConnection(PeerConnection *c)
{
    connect(c, SIGNAL(dataReady(int,QByteArray)), this, SLOT(dataFromPeerReady(int,QByteArray)));
    int i = currentID++;
    mPeerConnections.insert(i, c);
    c->setID(i);
    connect(c, SIGNAL(peerConnected(int)), this, SLOT(connectionSuccessful(int)));
}

Session & sessionHandler(ClientConsole & c, const QString & nick) {
	auto it = c.mNickConnectionMap.find(nick);
	int id = *it;
	auto it2 = c.mPeerSessions.find(id);
	return **it2;
}

void ClientConsole::dataFromPeerReady(int id, QByteArray a)
{
    QString parsedMessage;
    Messages::parseMessage(std::bind(sessionHandler, std::ref(*this), std::placeholders::_1), a, Messages::callbackHandler);
    qDebug() << "DATA supposed nickname(from connection) : " << mNickConnectionMap.key(id) << "MESSAGE: " << parsedMessage;
}



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
        qDebug() << "Unexpected input!";
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
				qDebug() << "selected peer not found";
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
}
