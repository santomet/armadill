#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include "common.h"

class ServerConnection; //we are going to use our certificate from here

class PeerConnection : public QObject
{
    Q_OBJECT
public:
    explicit PeerConnection(peer *mPeer, ServerConnection *server, QObject *parent = 0);

private:
    peer *mPeer;
    ServerConnection *mServer;


signals:

public slots:
};

#endif // PEERCONNECTION_H
