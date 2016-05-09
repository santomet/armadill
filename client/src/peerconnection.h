#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QtNetwork>
#include <QThread>
#include "common.h"
#include "messages.h"
#include "serverconnection.h"

class ServerConnection; //we are going to use our certificate from here


class PeerConnection : public QObject
{
    Q_OBJECT
public:
    explicit PeerConnection(qintptr soc = 0, peer mPeer = NULL, ServerConnection *server = nullptr, QObject *parent = 0);

    void sendData(QByteArray d);

    //getters
    int getID() {return mID;}
    peer getPeer() {return mPeer;}
    QTcpSocket *getSocket() {return mSoc;}
    bool isEstablished() {return mEstablished;}
    int getPeerPort() {return mPeerPort;}
    QString getPeerAddress() {return mPeerAddress;}
    //setters
    void setID(int id) {mID = id;}
    void setPeer(peer *p) {mPeer = p;}
    void establish() {mEstablished = true;}

private:
    QThread *mThread;
    int mID;
    peer mPeer;
    ServerConnection *mServer;

    QTcpSocket *mSoc;
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
    void readData();
private slots:
    void connectionError(QAbstractSocket::SocketError error);
    void connectionSuccess() {emit peerConnected(mID);}
    void disconnected() {emit peerDisconnected(mID);}
    void readDataFromClient() {emit dataReady(mID, mSoc->readAll());}

};

class ArmaTcpPeerServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ArmaTcpPeerServer(Messages *m, ServerConnection *s, QObject *parent) : QTcpServer(parent), mMessages(m),
        mServerConnection(s)
    {}


    QList<PeerConnection*> mConnections; //only connections initiated by other peer

signals:
    newRemoteInitiatedConnection(PeerConnection *c);
    endRemoteInitiatedConnection(PeerConnection *c);

public slots:
    void deleteConnection(PeerConnection *c) {mConnections.removeOne(c); emit endConnection(c);}
    void establishedConnection(PeerConnection *c) {disconnect(c, 0, this, 0); mConnections.removeOne(c);}
protected:
    void incomingConnection(qintptr handle) override {PeerConnection *c = new PeerConnection(handle, nullptr, mServerConnection);
                                                     mConnections.append(c);
                                                     emit newRemoteInitiatedConnection(c);
                                                     connect(c, SIGNAL(done(PeerConnection*)), this, SLOT(deleteConnection(PeerConnection*)), Qt::QueuedConnection);
                                                     }
    Messages *mMessages;
    ServerConnection *mServerConnection;
};
#endif // PEERCONNECTION_H
