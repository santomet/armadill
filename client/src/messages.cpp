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

	ret.append(key.protect(message.toUtf8(), ret));

	key.generateKey();
	return ret;
}

Messages::ArmaMessage Messages::createLoginMessage(QString & name, const QString & password) {
	ArmaMessage ret;
	ret.append(name.toUtf8());
	ret.append(password.toUtf8().toBase64());
	return ret;
}

bool Messages::parseMessage(Session &session, const ArmaMessage &message, Messages::ReceivedMessage &parsedMessage) {
	QByteArray senderNick, receiverNick, dh;
	QDateTime timestamp;
	short type;

	QList<QByteArray> list = message.split(armaSeparator);
	
	if (list.size() < 5) throw new MessageException("incomplete message");
	//senderNick = list[0];
	//receiverNick = list[1];

	timestamp.setMSecsSinceEpoch(list[2].toLongLong());
	parsedMessage.timestamp = timestamp;
	
	type = list[3].toShort();
	

	QByteArray messageText;
	QByteArray encryptedData;

	//regularMessage
	if (type == RegularMessage) {
		
		//in case of separator occurence in data
		encryptedData.append(list[4]);
		for (int i = 5; i < list.size(); i++) {
			encryptedData.append(armaSeparator);
			encryptedData.append(list[i]);
		}
		
		SessionKey& sk = session.getKey();
		int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + 4; //number of separators
		try{
			messageText = sk.unprotect(encryptedData, message.left(contextDataLength));
		}
		catch (KryptoException e) {
			return false;
		}
		parsedMessage.messageText = messageText;
	}

	//regularMessageDH
	if (type == RegularMessageDH) {
		if (list.size() < 6) throw new MessageException("incomplete message");
		dh = list[4];

		//in case of separator occurence in data
		encryptedData.append(list[5]);
		for (int i = 6; i < list.size(); i++) {
			encryptedData.append(armaSeparator);
			encryptedData.append(list[i]);
		}
		
		SessionKey& sk = session.getKey();
		sk.setDH(dh);
		int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + list[4].size() + 5;
		QByteArray messageText;
		try {
			messageText = sk.unprotect(encryptedData, message.left(contextDataLength));
		}
		catch (KryptoException e) {
			return false;
		}
		parsedMessage.messageText = messageText;
	}

	//fileTypes


	return true;
}
