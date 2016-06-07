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
#include "userinputhelper.h"

class ClientConsole : public QObject
{
    friend class UTest;
    Q_OBJECT
public:
    explicit ClientConsole(QStringList hostPort, QObject *parent = 0);


private:

    /**
     * @brief mInputHelper                  stdin reading cycle in separate thread
     */
    UserInputHelper *mInputHelper;

    /**
     * @brief mServerConnection             our connection to the server
     */
    ServerConnection *mServerConnection;

    mbedtls_entropy_context mTLS_entropy; //entropy

    int mActivePeer;    //peer which we are communicating with (no multiple chats)

    /**
     * @brief mPeerSessions, mPeerConnections, mNickConnectionMap               peer identifying system (sessions may not be used??)
     */

	int currentID = 0;
    QMap<int, Session*> mPeerSessions;
    QMap<int, PeerConnection*> mPeerConnections;
    QMap<QString, int> mNickConnectionMap;

    QStringList mServer;    //server properties from argument parser
    QSocketNotifier *mNotifier;         //obsolete notifier for unix stdin

    Messages *mMessages;        //this is now common everywhere. Originally intended to be one object per connection... somehow diverted

    /**
     * @brief mOnlinePeerList               list of logged in peers obtained form server
     */
    QList<peer> mOnlinePeerList;

    ArmaTcpPeerServer *mPeerServer;
    qint16 mListeningPort;

    /**
     * @brief The ExpectedUserInput enum    used to distinguish what we are expecting from user
     */
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

	void command(QString);

	friend Session & sessionHandler(ClientConsole & c, const QString &);
signals:
    void exitNormal();
    void sendDataToServer(QByteArray a);

    void sendDataToPeer(QByteArray a);
    void connectionEstablished(PeerConnection *c);

public slots:
    /**
     * @brief init                  initializes what sould be initialized inside the main eventloop
     */
    void init();
    //client-server
    void serverConnected() {mExpectedInput = LoginOrRegister;}
	void loginSuccess(QByteArray);
    void registrationSuccess() {qDebug() << "You can now log in (l) or register new account(r)"; mExpectedInput = LoginOrRegister;}
    void fail() {qDebug() << "Please try again - login(l) or register(r)"; mExpectedInput = LoginOrRegister;}
    void loggedInPeersFromServer(QByteArray a);

    //peer
    void connectToPeer(peer p);
    void connectionSuccessful(int id); //self-initiated
    void newRemoteInitiatedConnection(PeerConnection *c);
    void endConnection(PeerConnection *c) {} //remote-initiated before established
    void endConnection(int id) {} //self-initiated/established //TODO both

    void dataFromPeerReady(int id, QByteArray a);

    //peerServer
    void startPeerServer();



protected slots:
    void userInput(QString Qline);


};

#endif // CLIENTCONSOLE_H
