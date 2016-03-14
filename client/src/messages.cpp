#include "messages.h"

Messages::Messages(peer *peerToConnect, QObject *parent) : QObject(parent), mPeer(peerToConnect)
{
    //here we create peerconnection session
}

Messages::~Messages()
{

}

