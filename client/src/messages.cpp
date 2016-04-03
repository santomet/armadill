#include "messages.h"

Messages::Messages(peer *peerToConnect, QObject *parent) : QObject(parent), mPeer(peerToConnect)
{
    //here we create peerconnection session
}

Messages::~Messages()
{

}

Messages::ArmaMessage Messages::createRegularMessage(Session & session, const QString & message) {
	ArmaMessage ret;
	SessionKey & key = session.getKey();
	
	ret.append(session.getMyName());
	ret.append(Messages::armaSeparator);
	ret.append(session.getPartnerName());
	ret.append(Messages::armaSeparator);
	ret.append(QDateTime::currentMSecsSinceEpoch());
	ret.append(Messages::armaSeparator);
	
	if (!key.isMyDHCreated()) {
		ret.append('A' + Messages::RegularMessageDH);
		ret.append(Messages::armaSeparator);
		ret.append(key.getDH().toBase64());
	}
	else {
		ret.append('A' + Messages::RegularMessage);
	}
	ret.append(Messages::armaSeparator);

	ret.append(key.encrypt(message.toUtf8(), ret));

	key.generateKey();
	return ret;
}

