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
        mNotifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
        connect(mNotifier, SIGNAL(activated(int)), this, SLOT(userInput()));
        mMessages = new Messages(nullptr);
        mServerConnection = new ServerConnection(mServer.at(0), mServer.at(1).toInt());
        connect(mServerConnection, SIGNAL(connectSuccess()), this, SLOT(serverConnected()));
        connect(mServerConnection, SIGNAL(loginSuccess()), this, SLOT(loginSuccess()));
        connect(mServerConnection, SIGNAL(fail()), this, SLOT(fail()));
        connect(mServerConnection, SIGNAL(registrationSuccess()), this, SLOT(registrationSuccess()));
        connect(mServerConnection, SIGNAL(gotLoggedInPeers(QByteArray)), this, SLOT(loggedInPeersFromServer(QByteArray)));
        connect(this, SIGNAL(sendDataToServer(QByteArray)), mServerConnection, SLOT(sendDataToServer(QByteArray)));
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
}

void ClientConsole::userInput()
{
    std::string line;
    std::getline(std::cin, line);
    QString Qline = QString::fromStdString(line);
    //    qDebug() << Qline;

    switch (mExpectedInput)
    {
    case None :
        qDebug() << "Unexpected input!";
        break;
    case Idle :
        if(Qline == "p")
            emit sendDataToServer("j");
        //TODO
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
        emit sendDataToServer(mMessages->createLoginMessage(mNickName, mPassword, false));
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

