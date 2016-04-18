#ifndef CLIENTCONSOLE_H
#define CLIENTCONSOLE_H

#include <QDebug>
#include <QSocketNotifier>
#include <QCommandLineParser>
#include <QObject>
#include <QDataStream>
#include <QByteArray>
#include <QMap>
#include <iostream>
#include "messages.h"
#include "serverconnection.h"

class ClientConsole : public QObject
{
    friend class UTest;
    Q_OBJECT
public:
    explicit ClientConsole(QStringList hostPort, QObject *parent = 0);


private:

    QQueue<Messages::ArmaMessage*> messageQueue; //max 10

    ServerConnection *mServerConnection;
    QMap<int, Session*> mPeerSessions;
    QMap<int, PeerConnection*> mPeerConnections;

    QStringList mServer;
    QSocketNotifier *mNotifier;
    Messages *mMessages;

    QList<peer> mOnlinePeerList;


    enum ExpectedUserInput
    {
        None,
        Idle,   //after login - select clients, logout, blahblah
        LoginOrRegister,
        NickNameLogin,
        PasswordLogin,
        NickNameRegister,
        PasswordRegister,
        Message
    };

    ExpectedUserInput mExpectedInput{None};
    QString mNickName, mPassword;

signals:
    void exitNormal();
    void sendDataToServer(QByteArray a);

public slots:
    /**
     * @brief init                  initializes what sould be initialized inside the main eventloop
     */
    void init();
    void serverConnected() {mExpectedInput = LoginOrRegister;}
    void loginSuccess() {qDebug() << "You can load peers from server (p)"; mExpectedInput = Idle;}
    void registrationSuccess() {qDebug() << "You can now log in (l) or register new account(r)"; mExpectedInput = LoginOrRegister;}
    void fail() {qDebug() << "Please try again - login(l) or register(r)"; mExpectedInput = LoginOrRegister;}
    void peerConnected() {/*TODO*/}
    void loggedInPeersFromServer(QByteArray a);

protected slots:
    void userInput();


};

#endif // CLIENTCONSOLE_H
