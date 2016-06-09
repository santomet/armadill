#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QtNetwork>
#include <QThread>
#include <iostream>
#include "common.h"
#include "messages.h"
#include "serverconnection.h"

//class ServerConnection; //we are going to use our certificate from here


/**
 * @brief The PeerConnection class                      This class is managing (established) connection with another peer. Can be initiated by this peer (directly creating the socket)
 *                                                      or by remote peer (via server) All data construction/parsing for communication is made outside the class...
 */
class PeerConnection : public QObject
{
    Q_OBJECT
public:
    explicit PeerConnection(qintptr soc = 0, peer _peer = {}, ServerConnection *server = nullptr, QObject *parent = 0);

    //getters
    int getID() {return mID;}
    peer getPeer() {return mPeer;}
    QTcpSocket *getSocket() {return mSoc;}
    bool isEstablished() {return mEstablished;}
    int getPeerPort() {return mPeerPort;}
    QString getPeerAddress() {return mPeerAddress;}
    //setters
    void setID(int id) {mID = id;}
    void setPeer(peer p) {mPeer = p;}
    void establish() {mEstablished = true;}

private:
    QThread *mThread;
    int mID;
    peer mPeer;
    ServerConnection *mServer;

    QSslSocket *mSoc;
    qintptr mSocDescriptor;
    QString mPeerAddress;
    int mPeerPort;
    QString mNickName{""};

    bool mEstablished{false};

signals:
    void peerConnected(int id);
    void peerDisconnected(int id);
    void dataReady(int id, QByteArray a);

public slots:
    void init();
    void sendDataToPeer(QByteArray a);
    void successfulEncryptedConnection() {qDebug() << "Successful Encrypted Conenction";}
    void stateChanged(QAbstractSocket::SocketState s) {qDebug() << s;}
private slots:
    void connectionError(QAbstractSocket::SocketError error);
    void connectionSuccess() {emit peerConnected(mID);}
    void disconnected() {emit peerDisconnected(mID);}
    void readDataFromClient() {emit dataReady(mID, mSoc->readAll());}
	void sllErrorsClient(const QList<QSslError> & errors);
};


/**
 * @brief The ArmaTcpPeerServer class                   This is simple server designed to work with P2P. After identification of the remotely-connected peer that particular
 *                                                      socket is no longer maintained by this class
 */
class ArmaTcpPeerServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ArmaTcpPeerServer(ServerConnection *s, QObject *parent = 0) : QTcpServer(parent),
        mServerConnection(s)
    {}


    QList<PeerConnection*> mConnections; //only connections initiated by other peer

signals:
    void newRemoteInitiatedConnection(PeerConnection *c);
    void endRemoteInitiatedConnection(PeerConnection *c);

public slots:

    void deleteConnection(PeerConnection *c) {mConnections.removeOne(c); emit endRemoteInitiatedConnection(c);} //for ending the connection that has not been established yet
    void establishedConnection(PeerConnection *c) {disconnect(c, 0, this, 0); mConnections.removeOne(c);}   //when the connection is established (peer successfully identifies itself) we forget him
protected:
    void incomingConnection(qintptr handle) override {PeerConnection *c = new PeerConnection(handle, {}, mServerConnection); //creating PeerConnection with empty peer
                                                     mConnections.append(c);
                                                     emit newRemoteInitiatedConnection(c);
													 // TODO
                                                     //connect(c, SIGNAL(done(PeerConnection*)), this, SLOT(deleteConnection(PeerConnection*)), Qt::QueuedConnection);
                                                     }
    ServerConnection *mServerConnection;
};
#endif // PEERCONNECTION_H
