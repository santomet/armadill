#include "peerconnection.h"

PeerConnection::PeerConnection(peer *peerToConnect, ServerConnection *server, QObject *parent)
    : QObject(parent), mPeer(peerToConnect), mServer(server)
{
    //ssl connection initializes here
}

