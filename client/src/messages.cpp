#include "messages.h"

Messages::Messages(peer *peerToConnect = nullptr, QObject *parent) : QObject(parent), mPeer(peerToConnect)
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

Messages::ArmaMessage Messages::createLoginMessage(QString & name, const QString & password, int port, bool reg) {
    //TODO: certificate and port
	ArmaMessage ret;
    ret.append(reg ? "r" : "l");
    ret.append(Messages::armaSeparator);
	ret.append(name.toUtf8());
	ret.append(Messages::armaSeparator);
	ret.append(password.toUtf8().toBase64());
    ret.append(Messages::armaSeparator);
    ret.append(QString::number(port).toUtf8().toBase64());
	return ret;
}

bool Messages::isJsonMessage(const ArmaMessage& message) {
	return !QJsonDocument::fromJson(message).isNull();
}

bool Messages::parseJsonUsers(ArmaMessage &message, QList<peer>& usersList) {
	usersList.clear();
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message);
	if (jsonDoc.isNull()) {
		return false;
	}
	QJsonObject jsonObject = jsonDoc.object();
	if (jsonObject["users"].isArray()) {
		QJsonArray users = jsonObject["users"].toArray();
		foreach (QJsonValue user, users)
		{
			QJsonObject userObject = user.toObject();
			peer p;
			p.address = userObject["address"].toString();
			p.name = userObject["nick"].toString();
			p.listeningPort = userObject["port"].toInt();
			usersList.append(p);
		}
	}
	else {
		return false;
	}
	return true;
}

bool Messages::parseMessage(Session &session, ArmaMessage &message, Messages::ReceivedMessage &parsedMessage) {
    QByteArray senderNick, receiverNick, dh;
    QDateTime timestamp;
    char type;

    QList<QByteArray> list = message.split(armaSeparator);

    if (list.size() < 5) throw new MessageException("incomplete message");
    //senderNick = list[0];
    //receiverNick = list[1];

    timestamp.setMSecsSinceEpoch(list[2].toLongLong());
    parsedMessage.timestamp = timestamp;

    type = list[3][0] - 'A';


    QByteArray messageText;
    QByteArray encryptedData;

    //regularMessage
    if (type == RegularMessage) {

        SessionKey& sk = session.getKey();
        int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + 4; //number of separators
        try{
            QByteArray a1 = message.left(contextDataLength);
            QByteArray a2 = message.right(message.length() - contextDataLength);
            messageText = sk.unprotect(a2, a1);
        }
        catch (KryptoException e) {
            return false;
        }
        parsedMessage.messageText = messageText;
    }

    //regularMessageDH
    if (type == RegularMessageDH) {
        if (list.size() < 6) throw new MessageException("incomplete message");
        dh = QByteArray::fromBase64(list[4]);


        SessionKey& sk = session.getKey();

        int contextDataLength = list[0].size() + list[1].size() + list[2].size() + list[3].size() + list[4].size() + 5;
        QByteArray messageText;
        try {
            QByteArray a1 = message.left(contextDataLength);
            QByteArray a2 = message.right(message.length() - contextDataLength);
            messageText = sk.unprotect(a2, a1);
        }
        catch (KryptoException e) {
            return false;
        }
        sk.setDH(dh);
        parsedMessage.messageText = messageText;
    }

    //fileTypes


    return true;
}
