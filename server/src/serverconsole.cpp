#include "serverconsole.h"

ServerConsole::ServerConsole(QCommandLineParser *arguments, QObject *parent) : QObject(parent),
    mArguments(arguments)
{

}

void ServerConsole::init()
{
    if(mArguments->positionalArguments().count() != 1 || !mArguments->isSet("database"))
    {
        qDebug() << "invalid use, look at -h";
        QCoreApplication::exit(1);
    }
    else
    {
        mServerManager = new ServerManager(mArguments->value("database"));
        mPort = mArguments->positionalArguments().at(0).toInt();
        mTcpServer = new ArmaTcpServer(mServerManager, this);
        if(!mTcpServer->listen(QHostAddress::Any, mPort))
        {
            qDebug() << "Error while starting a server: " << mTcpServer->errorString();
            QCoreApplication::exit(1);
        }
        else
        {
            qDebug() << "Server Started...";
        }
    }
}

