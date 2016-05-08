#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QtNetwork>
#include "common.h"

class ServerConnection; //we are going to use our certificate from here

class PeerConnection : public QObject
{
    Q_OBJECT
public:
    explicit PeerConnection(peer *mPeer, ServerConnection *server = nullptr, QObject *parent = 0);

    void sendData(QByteArray d);

    //getters
    peer *getPeer() {return mPeer;}

private:
    peer *mPeer;
    ServerConnection *mServer;


signals:

public slots:
};

#endif // PEERCONNECTION_H
