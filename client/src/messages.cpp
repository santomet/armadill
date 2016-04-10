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
	
	ret.append(session.getMyName().toUtf8().toBase64());
	ret.append(Messages::armaSeparator);
	ret.append(session.getPartnerName().toUtf8().toBase64());
	ret.append(Messages::armaSeparator);
	ret.append(QString::number(QDateTime::currentMSecsSinceEpoch()));
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

bool Messages::parseMessage(Session &session, ArmaMessage &message, Messages::ReceivedMessage &parsedMessage) {
	QByteArray senderNick, receiverNick, dh;
	QDateTime timestamp;
	char type;
	
	int separatorCount = message.count(armaSeparator);
	if (separatorCount < 4) throw new MessageException("incomplete message");
	int* position = new int[separatorCount]; // positions of separators # in sender#receiver#time#type#
	
	position[0] = message.indexOf(armaSeparator);
	for (int i = 1; position[i-1] != -1; i++) {
		position[i] = message.indexOf(armaSeparator, position[i - 1] + 1);
	}
	
	senderNick = QByteArray::fromBase64(message.left(position[0]));
	receiverNick = QByteArray::fromBase64(message.mid(position[0] + 1, position[1] - position[0] - 1));
	timestamp.setMSecsSinceEpoch(message.mid(position[1] + 1, position[2] - position[1] - 1).toLongLong());
	parsedMessage.timestamp = timestamp;
	type = message[position[2] + 1] - 'A';

	QByteArray messageText;
	
	//regularMessage
	if (type == RegularMessage) {
		SessionKey& sk = session.getKey();
		try{
			messageText = sk.unprotect(message.mid(position[3] + 1), message.left(position[3] + 1));
		}
		catch (KryptoException e) {
			return false;
		}
		parsedMessage.messageText = messageText;
	}

	//regularMessageDH
	if (type == RegularMessageDH) {
		if (separatorCount < 5) throw new MessageException("incomplete message");
		dh = QByteArray::fromBase64(message.mid(position[3] + 1, position[4] - position[3] - 1));
	
		SessionKey& sk = session.getKey();

		try {
			messageText = sk.unprotect(message.mid(position[4] + 1), message.left(position[4] + 1));
		}
		catch (KryptoException e) {
			return false;
		}
		sk.setDH(dh);
		parsedMessage.messageText = messageText;
	}

	//fileTypes

	//BUG: problem with dealocating memory
	//delete[] position;
	
	return true;
}
